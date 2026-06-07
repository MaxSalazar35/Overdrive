// ============================================================
// Jugador.cpp — Overdrive
// Salto estilo Mario: fuerza sostenida mientras se presiona,
// con límite de tiempo. Cooldown de 1.5s entre saltos.
// Desliz: dura mientras el botón esté presionado y haya velocidad.
//
// Animaciones fluidas:
//  - Salto:   frames 0..3 durante la subida, se congela en frame 3 al caer.
//             Al aterrizar vuelve a idle/correr suavemente.
//  - Desliz:  animación loop mientras se desliza. Al soltar, congela el
//             último frame DURACION_SALIDA_DESLIZ seg antes de transicionar.
//  - Correr:  loop estándar; al empezar a correr no hace "pop" porque
//             setEstado solo reinicia si cambia de estado.
// ============================================================
#include "Jugador.hpp"
#include <cmath>
#include <iostream>

static const int FW = 200;
static const int FH = 220;

// ── Movimiento horizontal ─────────────────────────────────────
static const float VEL_MAX     = 400.f;
static const float ACELERACION = 220.f;
static const float FRICCION    = 0.80f;
static const float FRIC_AIRE   = 0.92f;

// ── Salto estilo Mario ────────────────────────────────────────
static const float SALTO_VEL        = -400.f;
static const float SALTO_TIEMPO_MAX =  0.50f;
static const float SALTO_COOLDOWN   =  1.5f;

// ── Desliz ────────────────────────────────────────────────────
static const float DESLIZ_FRICCION  = 0.03f;
static const float DESLIZ_VEL_MIN   = 10.f;

Jugador::Jugador(b2WorldId worldId, sf::Vector2f pos,
                 int id, SistemaParticulas& parts,
                 const std::string& rutaSprite,
                 sf::Color colorPj)
    : id(id), estado(EstadoJug::Cayendo), estadoAnterior(EstadoJug::Cayendo),
      enSuelo(false), mirandoDerecha(true),
      tiempoSinSuelo(0.f), ganchoPresionado(false),
      deslizPresionado(false), timerDesliz(0.f),
      timerSalidaDesliz(0.f),
      animCaidaCongelada(false),
      saltando(false), puedesSaltar(false),
      timerSalto(0.f), cooldownSalto(0.f),
      deslizBoostAplicado(false),
      worldId(worldId), input(id), colorPersonaje(colorPj),
      particulas(parts)
{
    anim.configurar(FW, FH);
    anim.agregarEstado(ANIM_IDLE,     {0, 4, 0.18f, true});
    anim.agregarEstado(ANIM_CORRER,   {1, 6, 0.09f, true});
    anim.agregarEstado(ANIM_SALTAR,   {2, 4, 0.12f, false});
    anim.agregarEstado(ANIM_DESLIZAR, {3, 2, 0.15f, true});

    // Usar la ruta pasada; si está vacía usar el default por id
    std::string ruta = rutaSprite.empty()
        ? (id == 0 ? "assets/images/vandal.png" : "assets/images/cobalt.png")
        : rutaSprite;
    if (!textura.loadFromFile(ruta))
        std::cerr << "[Jugador] No se cargo " << ruta << "\n";

    sprite.setTexture(textura);
    sprite.setScale(0.45f, 0.45f);
    sprite.setOrigin(FW / 2.f, FH / 2.f);

    b2BodyDef bd     = b2DefaultBodyDef();
    bd.type          = b2_dynamicBody;
    bd.position      = {pos.x, pos.y};
    bd.fixedRotation = true;
    bd.linearDamping = 0.f;
    cuerpo = b2CreateBody(worldId, &bd);

    b2Polygon box  = b2MakeBox(20.f, 35.f);
    b2ShapeDef sd  = b2DefaultShapeDef();
    sd.density     = 1.f;
    sd.friction    = 0.f;
    sd.restitution = 0.f;
    b2CreatePolygonShape(cuerpo, &sd, &box);

    gancho = new Gancho(worldId, cuerpo);
}

Jugador::~Jugador()
{
    delete gancho;
    if (b2Body_IsValid(cuerpo))
        b2DestroyBody(cuerpo);
}

// ── Detección de suelo ───────────────────────────────────────
static bool detectarSuelo(b2WorldId worldId, b2BodyId cuerpo)
{
    b2Vec2 pos = b2Body_GetPosition(cuerpo);
    b2Vec2 ori = {pos.x, pos.y + 33.f};
    b2Vec2 fin = {pos.x, pos.y + 43.f};

    struct Ctx { bool toca = false; };
    Ctx ctx;

    auto cb = [](b2ShapeId sid, b2Vec2, b2Vec2, float frac, void* raw) -> float {
        if (b2Shape_IsSensor(sid)) return -1.f;
        if (b2Body_GetType(b2Shape_GetBody(sid)) == b2_dynamicBody) return -1.f;
        static_cast<Ctx*>(raw)->toca = true;
        return frac;
    };

    b2World_CastRay(worldId, ori, fin, b2DefaultQueryFilter(), cb, &ctx);
    return ctx.toca;
}

// ── Entrada ─────────────────────────────────────────────────
void Jugador::procesarEntrada(float dt)
{
    if (estado == EstadoJug::Muerto) return;
    input.update();

    bool botonDesliz = input.presionado(Accion::Deslizar);

    // ── Deslizamiento activo ──────────────────────────────────
    if (timerDesliz > 0.f) {
        b2Vec2 va  = b2Body_GetLinearVelocity(cuerpo);
        float velX = va.x * (1.f - DESLIZ_FRICCION * dt * 60.f);
        if (std::abs(velX) < 5.f) velX = 0.f;
        b2Body_SetLinearVelocity(cuerpo, {velX, va.y});

        if (!botonDesliz || !enSuelo) {
            // Solto el boton o salio del suelo: iniciar salida animada
            timerDesliz       = 0.f;
            timerSalidaDesliz = DURACION_SALIDA_DESLIZ;
        }

        ganchoPresionado = true;
        deslizPresionado = botonDesliz;
        return;
    }

    // ── Horizontal ───────────────────────────────────────────
    float velX = b2Body_GetLinearVelocity(cuerpo).x;
    float eje  = input.ejeX();

    if (eje > 0.1f) {
        velX += ACELERACION * dt;
        if (velX >  VEL_MAX) velX =  VEL_MAX;
        mirandoDerecha = true;
    } else if (eje < -0.1f) {
        velX -= ACELERACION * dt;
        if (velX < -VEL_MAX) velX = -VEL_MAX;
        mirandoDerecha = false;
    } else {
        velX *= enSuelo ? FRICCION : FRIC_AIRE;
        if (std::abs(velX) < 5.f) velX = 0.f;
    }

    // ── Salto estilo Mario ────────────────────────────────────
    bool botonSalto = input.presionado(Accion::Saltar);

    if (botonSalto && !saltando && enSuelo && puedesSaltar && cooldownSalto <= 0.f) {
        saltando              = true;
        timerSalto            = SALTO_TIEMPO_MAX;
        puedesSaltar          = false;
        animCaidaCongelada    = false;  // reiniciar flag para esta nueva subida
        particulas.emitir(getPosicion() + sf::Vector2f(0, 35.f),
                          TipoParticula::Polvo, sf::Color::White, 8);
    }

    if (!botonSalto && saltando) {
        saltando      = false;
        timerSalto    = 0.f;
        cooldownSalto = SALTO_COOLDOWN;
    }

    if (saltando && timerSalto > 0.f) {
        timerSalto -= dt;
        if (timerSalto <= 0.f) {
            timerSalto    = 0.f;
            saltando      = false;
            cooldownSalto = SALTO_COOLDOWN;
        } else {
            b2Body_SetLinearVelocity(cuerpo, {velX, SALTO_VEL});
            goto salto_gancho;
        }
    }

    // Solo X — Y la maneja Box2D
    {
        b2Vec2 va = b2Body_GetLinearVelocity(cuerpo);
        b2Body_SetLinearVelocity(cuerpo, {velX, va.y});
    }

    // ── Deslizamiento (activar) ───────────────────────────────
    {
        bool iniciarDesliz = botonDesliz && !deslizPresionado
                             && enSuelo
                             && std::abs(velX) > DESLIZ_VEL_MIN;
        deslizPresionado = botonDesliz;
        if (iniciarDesliz) {
            timerDesliz         = 99.f;
            timerSalidaDesliz   = 0.f;
            deslizBoostAplicado = false;   // resetear para esta nueva deslizada
            particulas.emitir(getPosicion() + sf::Vector2f(0, 35.f),
                              TipoParticula::Polvo, sf::Color::White, 12);
        }
    }

    // ── Boost de desliz en bajada ─────────────────────────────
    // Si el jugador está deslizando y la plataforma "baja" (detectamos
    // inclinación revisando si hay suelo un poco más adelante y más abajo),
    // aplicamos un impulso extra UNA sola vez por deslizada.
    // Como las plataformas son horizontales usamos un enfoque simple:
    // si el jugador desliza y su velocidad Y reciente era positiva antes
    // de tocar el suelo (venía cayendo), es "bajada". Lo simplificamos:
    // si el jugador supera cierta velocidad X al deslizar, reforzamos.
    if (timerDesliz > 0.f && !deslizBoostAplicado && enSuelo) {
        b2Vec2 va = b2Body_GetLinearVelocity(cuerpo);
        float absVel = std::abs(va.x);
        if (absVel > 80.f) {
            // Impulso extra: 15% de la velocidad actual en la dirección de movimiento
            float boost = va.x * 0.15f;
            // Cap para que no se dispare
            float newVelX = va.x + boost;
            float cap = 480.f;
            if (newVelX >  cap) newVelX =  cap;
            if (newVelX < -cap) newVelX = -cap;
            b2Body_SetLinearVelocity(cuerpo, {newVelX, va.y});
            deslizBoostAplicado = true;
        }
    }

salto_gancho:
    // ── Gancho ───────────────────────────────────────────────
    bool botonGancho = input.presionado(Accion::Gancho);
    if (botonGancho && !ganchoPresionado) {
        // El ángulo se calcula internamente en Gancho según la velocidad actual.
        sf::Vector2f dir(0.f, -1.f);
        bool ok = gancho->disparar(dir, mirandoDerecha);
        if (ok)
            particulas.emitir(gancho->getPuntoAnclaje(),
                              TipoParticula::Chispa, sf::Color(255,220,80), 12);
    }
    ganchoPresionado = botonGancho;
}

// ── Update ───────────────────────────────────────────────────
void Jugador::update(float dt)
{
    if (estado == EstadoJug::Muerto) return;

    bool teniaSuelo = enSuelo;
    enSuelo = detectarSuelo(worldId, cuerpo);

    if (!enSuelo) {
        tiempoSinSuelo += dt;
        if (timerDesliz > 0.f) timerDesliz = 0.f;
    } else {
        tiempoSinSuelo = 0.f;
        puedesSaltar   = true;
        animCaidaCongelada = false;   // ya tocó suelo, listo para el próximo salto
        if (!teniaSuelo)
            particulas.emitir(getPosicion() + sf::Vector2f(0, 35.f),
                              TipoParticula::Polvo, sf::Color::White, 10);
    }

    if (cooldownSalto > 0.f) {
        cooldownSalto -= dt;
        if (cooldownSalto < 0.f) cooldownSalto = 0.f;
    }

    // Timer de salida del desliz (congela el último frame)
    if (timerSalidaDesliz > 0.f) {
        timerSalidaDesliz -= dt;
        if (timerSalidaDesliz < 0.f) timerSalidaDesliz = 0.f;
    }

    gancho->update(dt);
    actualizarEstado();
    actualizarAnimacion();

    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    if (std::abs(vel.x) > 280.f
        && relojEstela.getElapsedTime().asSeconds() > 0.05f) {
        sf::Color c = colorPersonaje; c.a = 140;
        particulas.emitir(getPosicion(), TipoParticula::Estela, c, 2);
        relojEstela.restart();
    }
}

// ── Draw ─────────────────────────────────────────────────────
void Jugador::draw(sf::RenderWindow& ventana)
{
    if (estado == EstadoJug::Muerto) return;

    sf::Color cc = colorPersonaje; cc.a = 200;
    gancho->draw(ventana, cc);

    b2Vec2 pos = b2Body_GetPosition(cuerpo);

    if (textura.getSize().x > 0) {
        sprite.setPosition(pos.x, pos.y);
        sprite.setTextureRect(anim.getRect());
        float sx = 0.45f;
        sprite.setScale(mirandoDerecha ? sx : -sx, 0.45f);
        ventana.draw(sprite);
    } else {
        sf::RectangleShape box({40.f, 70.f});
        box.setOrigin(20.f, 35.f);
        box.setPosition(pos.x, pos.y);
        sf::Color boxColor = colorPersonaje; boxColor.a = 200;
        box.setFillColor(boxColor);
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        ventana.draw(box);
    }
}

// ── Getters ──────────────────────────────────────────────────
sf::Vector2f Jugador::getPosicion() const {
    b2Vec2 p = b2Body_GetPosition(cuerpo);
    return {p.x, p.y};
}
float Jugador::getVelocidadX() const {
    return b2Body_GetLinearVelocity(cuerpo).x;
}
sf::FloatRect Jugador::getBounds() const {
    sf::Vector2f p = getPosicion();
    return {p.x-20.f, p.y-35.f, 40.f, 70.f};
}

void Jugador::eliminar() {
    estado = EstadoJug::Muerto;
    gancho->soltar();
    b2Body_Disable(cuerpo);
}

void Jugador::resetear(sf::Vector2f pos) {
    estado              = EstadoJug::Cayendo;
    estadoAnterior      = EstadoJug::Cayendo;
    enSuelo             = false;
    tiempoSinSuelo      = 0.f;
    ganchoPresionado    = false;
    deslizPresionado    = false;
    timerDesliz         = 0.f;
    timerSalidaDesliz   = 0.f;
    animCaidaCongelada  = false;
    saltando            = false;
    puedesSaltar        = false;
    timerSalto          = 0.f;
    cooldownSalto       = 0.f;
    deslizBoostAplicado = false;
    mirandoDerecha      = (id == 0);
    gancho->soltar();
    b2Body_Enable(cuerpo);
    b2Body_SetTransform(cuerpo, {pos.x, pos.y}, b2MakeRot(0.f));
    b2Body_SetLinearVelocity(cuerpo, {0.f, 0.f});
}

void Jugador::recibirImpacto() {
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    b2Body_SetLinearVelocity(cuerpo, {vel.x * 0.2f, -300.f});
    gancho->soltar();
}

void Jugador::actualizarEstado() {
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);

    estadoAnterior = estado;

    if (timerDesliz > 0.f) {
        estado = EstadoJug::Deslizando;
        return;
    }
    // Durante la salida del desliz mantenemos el estado Deslizando
    // para que actualizarAnimacion pueda mostrar el frame congelado.
    if (timerSalidaDesliz > 0.f) {
        estado = EstadoJug::Deslizando;
        return;
    }
    if (enSuelo) {
        float absVel = std::abs(vel.x);
        if (estado == EstadoJug::Corriendo)
            estado = absVel > 15.f ? EstadoJug::Corriendo : EstadoJug::Idle;
        else
            estado = absVel > 60.f ? EstadoJug::Corriendo : EstadoJug::Idle;
    } else {
        estado = vel.y < 0.f ? EstadoJug::Saltando : EstadoJug::Cayendo;
    }
}

void Jugador::actualizarAnimacion() {
    bool congelar = false;

    switch (estado) {
        case EstadoJug::Idle:
            anim.setEstado(ANIM_IDLE);
            break;

        case EstadoJug::Corriendo:
            anim.setEstado(ANIM_CORRER);
            break;

        case EstadoJug::Saltando:
            // loop=false: avanza automáticamente y se detiene en el último frame.
            // setEstado solo reinicia si cambia desde otro estado (por ej. desde Idle/Corriendo).
            anim.setEstado(ANIM_SALTAR);
            break;

        case EstadoJug::Cayendo:
            // Usamos la misma fila de salto. Nos aseguramos de estar en ANIM_SALTAR.
            anim.setEstado(ANIM_SALTAR);
            // La primera vez que entramos en Cayendo congelamos en el último frame
            // y mantenemos esa congelación mientras seguimos cayendo.
            congelar = true;
            if (!animCaidaCongelada) {
                animCaidaCongelada = true;
            }
            break;

        case EstadoJug::Deslizando:
            anim.setEstado(ANIM_DESLIZAR);
            if (timerSalidaDesliz > 0.f) {
                // Fase de salida: congela el último frame hasta que el timer expire.
                congelar = true;
            }
            break;

        default:
            break;
    }

    // Avanzar la animación normalmente…
    anim.update();
    // …y después congelar si corresponde (así el freeze siempre gana).
    if (congelar) {
        anim.congelarUltimoFrame();
    }
}

// ── Freno externo (por colisión con caja del rival) ──────────
void Jugador::aplicarFreno(float factor)
{
    if (estado == EstadoJug::Muerto) return;
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    // Solo frenar si hay velocidad significativa
    if (std::abs(vel.x) > 20.f) {
        b2Body_SetLinearVelocity(cuerpo, {vel.x * factor, vel.y});
        // Cancelar desliz si estaba activo (el golpe lo interrumpe)
        if (timerDesliz > 0.f) {
            timerDesliz       = 0.f;
            timerSalidaDesliz = DURACION_SALIDA_DESLIZ;
        }
        // Partículas de impacto
        particulas.emitir(getPosicion() + sf::Vector2f(0, 10.f),
                          TipoParticula::Polvo, sf::Color(255, 200, 80), 8);
    }
}
