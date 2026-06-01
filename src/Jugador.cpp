// ============================================================
// Jugador.cpp — Overdrive
// Simple y directo. Fisica manual sobre Box2D v3.
// ============================================================
#include "Jugador.hpp"
#include <cmath>
#include <iostream>

// Spritesheet: 1200x880, celdas 200x220
// Fila 0 = idle (4f), Fila 1 = correr (6f)
// Fila 2 = saltar (4f), Fila 3 = deslizar (2f)
static const int FW = 200;
static const int FH = 220;

// Velocidades
static const float VEL_MAX    = 400.f;
static const float ACELERACION= 220.f;  // px/s por frame
static const float FRICCION   = 0.80f;  // factor por frame en suelo
static const float FRIC_AIRE  = 0.92f;
static const float IMPULSO_SALTO = -700.f;  // calibrado para gravedad 2450 (~100px de altura)

Jugador::Jugador(b2WorldId worldId, sf::Vector2f pos,
                 int id, SistemaParticulas& parts)
    : id(id), estado(EstadoJug::Cayendo),
      enSuelo(false), mirandoDerecha(true),
      tiempoSinSuelo(0.f), saltoPresionado(false), ganchoPresionado(false),
      worldId(worldId), input(id), particulas(parts)
{
    // ── Animacion ────────────────────────────────────────────
    anim.configurar(FW, FH);
    anim.agregarEstado(ANIM_IDLE,     {0, 4, 0.18f, true});
    anim.agregarEstado(ANIM_CORRER,   {1, 6, 0.09f, true});
    anim.agregarEstado(ANIM_SALTAR,   {2, 4, 0.12f, false});
    anim.agregarEstado(ANIM_DESLIZAR, {3, 2, 0.15f, true});

    // ── Sprite ───────────────────────────────────────────────
    std::string ruta = (id == 0) ? "assets/images/jugador1.png"
                                 : "assets/images/jugador2.png";
    if (!textura.loadFromFile(ruta))
        std::cerr << "[Jugador] No se cargo " << ruta << "\n";

    sprite.setTexture(textura);
    // El personaje dentro de la celda mide ~120x180 px
    // Queremos que se vea ~90x130 en pantalla
    sprite.setScale(0.45f, 0.45f);
    sprite.setOrigin(FW / 2.f, FH / 2.f);

    // ── Cuerpo Box2D ─────────────────────────────────────────
    b2BodyDef bd    = b2DefaultBodyDef();
    bd.type         = b2_dynamicBody;
    bd.position     = {pos.x, pos.y};
    bd.fixedRotation= true;
    bd.linearDamping= 0.f;
    cuerpo = b2CreateBody(worldId, &bd);

    b2Polygon box  = b2MakeBox(20.f, 35.f);   // hitbox 40x70 px
    b2ShapeDef sd  = b2DefaultShapeDef();
    sd.density     = 1.f;
    sd.friction    = 0.f;                      // sin friccion lateral
    sd.restitution = 0.f;
    b2CreatePolygonShape(cuerpo, &sd, &box);

    // ── Gancho ───────────────────────────────────────────────
    gancho = new Gancho(worldId, cuerpo);
}

Jugador::~Jugador()
{
    delete gancho;
    if (b2Body_IsValid(cuerpo))
        b2DestroyBody(cuerpo);
}

// ── Deteccion de suelo (raycast 10px hacia abajo) ──────────
static bool detectarSuelo(b2WorldId worldId, b2BodyId cuerpo)
{
    b2Vec2 pos = b2Body_GetPosition(cuerpo);
    b2Vec2 ori = {pos.x, pos.y + 33.f};   // un poco antes del borde inferior
    b2Vec2 fin = {pos.x, pos.y + 43.f};   // 10px mas abajo

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

// ── Entrada ────────────────────────────────────────────────
void Jugador::procesarEntrada(float dt)
{
    if (estado == EstadoJug::Muerto) return;
    input.update();

    // Leer velocidad actual de Box2D
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);

    // ── Horizontal: solo modificamos X ───────────────────────
    float eje = input.ejeX();
    if (eje > 0.1f) {
        vel.x += ACELERACION * dt;
        if (vel.x > VEL_MAX) vel.x = VEL_MAX;
        mirandoDerecha = true;
    } else if (eje < -0.1f) {
        vel.x -= ACELERACION * dt;
        if (vel.x < -VEL_MAX) vel.x = -VEL_MAX;
        mirandoDerecha = false;
    } else {
        vel.x *= enSuelo ? FRICCION : FRIC_AIRE;
        if (std::abs(vel.x) < 5.f) vel.x = 0.f;
    }

    // ── Salto ────────────────────────────────────────────────
    // IMPORTANTE: usamos un bool de NIVEL para el salto,
    // no la tecla directamente, para que solo salte UNA VEZ
    // por pulsación (edge trigger, no level trigger)
    bool botonSaltoAhora = input.presionado(Accion::Saltar);
    bool saltarEsteFrame = botonSaltoAhora && !saltoPresionado && enSuelo;
    saltoPresionado = botonSaltoAhora;

    if (saltarEsteFrame) {
        vel.y = IMPULSO_SALTO;
        particulas.emitir(getPosicion() + sf::Vector2f(0,35.f),
                          TipoParticula::Polvo, sf::Color::White, 8);
    }

    // UN SOLO SetLinearVelocity con X e Y ya calculados
    b2Body_SetLinearVelocity(cuerpo, {vel.x, vel.y});

    // ── Gancho ────────────────────────────────────────────────
    bool botonGancho = input.presionado(Accion::Gancho);
    if (botonGancho && !ganchoPresionado) {
        constexpr float PI = 3.14159265f;
        float ang = mirandoDerecha ? -0.4f : PI + 0.4f;
        sf::Vector2f dir(std::cos(ang), std::sin(ang));
        bool ok = gancho->disparar(dir);
        if (ok)
            particulas.emitir(gancho->getPuntoAnclaje(),
                              TipoParticula::Chispa, sf::Color(255,220,80), 10);
    }
    ganchoPresionado = botonGancho;
}

// ── Update ─────────────────────────────────────────────────
void Jugador::update(float dt)
{
    if (estado == EstadoJug::Muerto) return;

    bool teniaSuelo = enSuelo;
    enSuelo = detectarSuelo(worldId, cuerpo);

    if (!enSuelo) tiempoSinSuelo += dt;
    else          tiempoSinSuelo  = 0.f;

    if (enSuelo && !teniaSuelo)   // aterrizaje
        particulas.emitir(getPosicion() + sf::Vector2f(0,35.f),
                          TipoParticula::Polvo, sf::Color::White, 10);

    gancho->update(dt);
    actualizarEstado();
    actualizarAnimacion();

    // Estela a alta velocidad
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    if (std::abs(vel.x) > 280.f
        && relojEstela.getElapsedTime().asSeconds() > 0.05f) {
        sf::Color c = id==0 ? sf::Color(80,180,255,140)
                            : sf::Color(255,100,60,140);
        particulas.emitir(getPosicion(), TipoParticula::Estela, c, 2);
        relojEstela.restart();
    }
}

// ── Draw ───────────────────────────────────────────────────
void Jugador::draw(sf::RenderWindow& ventana)
{
    if (estado == EstadoJug::Muerto) return;

    // Cuerda del gancho
    sf::Color cc = id==0 ? sf::Color(80,200,255,200)
                         : sf::Color(255,100,80,200);
    gancho->draw(ventana, cc);

    b2Vec2 pos = b2Body_GetPosition(cuerpo);

    if (textura.getSize().x > 0) {
        sprite.setPosition(pos.x, pos.y);
        sprite.setTextureRect(anim.getRect());
        float sx = 0.45f;
        sprite.setScale(mirandoDerecha ? sx : -sx, 0.45f);
        ventana.draw(sprite);
    } else {
        // Placeholder si no cargo la textura
        sf::RectangleShape box({40.f, 70.f});
        box.setOrigin(20.f, 35.f);
        box.setPosition(pos.x, pos.y);
        box.setFillColor(id==0 ? sf::Color(80,160,255,200)
                                : sf::Color(255,80,80,200));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);
        ventana.draw(box);
    }
}

// ── Getters / setters ──────────────────────────────────────
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
    estado          = EstadoJug::Cayendo;
    enSuelo         = false;
    tiempoSinSuelo  = 0.f;
    saltoPresionado = false;
    ganchoPresionado= false;
    mirandoDerecha  = (id == 0);
    gancho->soltar();
    b2Body_Enable(cuerpo);
    b2Body_SetTransform(cuerpo, {pos.x, pos.y}, b2MakeRot(0.f));
    b2Body_SetLinearVelocity(cuerpo, {0.f, 0.f});
}

void Jugador::recibirImpacto() {
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    b2Body_SetLinearVelocity(cuerpo, {vel.x*0.2f, -300.f});
    gancho->soltar();
}

void Jugador::actualizarEstado() {
    b2Vec2 vel = b2Body_GetLinearVelocity(cuerpo);
    if (enSuelo)
        estado = std::abs(vel.x) > 30.f ? EstadoJug::Corriendo : EstadoJug::Idle;
    else
        estado = vel.y < 0.f ? EstadoJug::Saltando : EstadoJug::Cayendo;
}

void Jugador::actualizarAnimacion() {
    switch (estado) {
        case EstadoJug::Idle:       anim.setEstado(ANIM_IDLE);     break;
        case EstadoJug::Corriendo:  anim.setEstado(ANIM_CORRER);   break;
        case EstadoJug::Saltando:
        case EstadoJug::Cayendo:    anim.setEstado(ANIM_SALTAR);   break;
        case EstadoJug::Deslizando: anim.setEstado(ANIM_DESLIZAR); break;
        default: break;
    }
    anim.update();
}
