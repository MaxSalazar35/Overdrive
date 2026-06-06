// ============================================================
// Gancho.cpp — Overdrive (Box2D v3)
//
// MECÁNICA:
//  - Disparo diagonal: ángulo GANCHO_ANGULO (70°) hacia arriba-adelante
//    según la dirección que mira el jugador. Sin límite horizontal.
//  - Sin filtro de Y: se ancla en cualquier superficie estática bajo el rayo.
//  - Al anclar: DistanceJoint como péndulo + impulso retráctil continuo
//    que jala al jugador hacia el punto de anclaje (GANCHO_RETRACCION px/s).
//  - Longitud mínima GANCHO_LONG_MIN: el jugador queda cerca del techo
//    y sale disparado con la velocidad acumulada al soltar.
// ============================================================
#include "Gancho.hpp"
#include <cmath>

// ── Raycast callback — cualquier superficie estática (no dinámica, no sensor) ──────
static float raycastCallback(b2ShapeId shapeId, b2Vec2 point,
                              b2Vec2 normal, float fraction, void* context)
{
    RaycastCtx* ctx = static_cast<RaycastCtx*>(context);
    if (b2Shape_IsSensor(shapeId)) return -1.f;
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_GetType(bodyId) == b2_dynamicBody) return -1.f;

    // Doble filtro para anclar SOLO en el techo continuo (y=0..20):
    //  1. normal.y < -0.5 → cara superior (elimina paredes y suelos)
    //  2. point.y < 30    → muy cerca del borde superior de la ventana (elimina
    //                       rampas y plataformas, que están en y >= 200)
    if (normal.y > -0.5f) return -1.f;   // no es cara superior
    if (point.y  > 30.f)  return -1.f;   // no es el techo continuo

    ctx->golpeo         = true;
    ctx->puntoContacto  = point;
    ctx->cuerpoGolpeado = bodyId;
    return fraction;
}

// ── Constructor ─────────────────────────────────────────────
Gancho::Gancho(b2WorldId worldId, b2BodyId cuerpoJugador)
    : worldId(worldId), cuerpoJugador(cuerpoJugador)
{
    punta.setRadius(6.f); punta.setOrigin(6.f, 6.f);
    punta.setFillColor(sf::Color(255, 220, 80));
    punta.setOutlineColor(sf::Color(255, 255, 180));
    punta.setOutlineThickness(2.f);
    lineaCuerda.setPrimitiveType(sf::Lines);
    lineaCuerda.resize(2);
}

Gancho::~Gancho() { destruirCuerda(); }

// ── disparar ─────────────────────────────────────────────────
// mirandoDerecha determina el sesgo horizontal del rayo.
bool Gancho::disparar(sf::Vector2f /*dir*/, bool mirandoDerecha)
{
    // Si ya está anclado, soltar
    if (estado == EstadoGancho::Anclado) { soltar(); return false; }

    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);

    // Ángulo diagonal: 70° desde horizontal, sesgado hacia donde mira el jugador.
    // cos(70°) ≈ 0.342 (componente horizontal), sin(70°) ≈ 0.940 (componente vertical)
    float dirX = mirandoDerecha ?  std::cos(GANCHO_ANGULO)
                                : -std::cos(GANCHO_ANGULO);
    float dirY = -std::sin(GANCHO_ANGULO);  // negativo = hacia arriba en coordenadas SFML

    b2Vec2 origen  = posJ;
    b2Vec2 destino = {posJ.x + dirX * GANCHO_LONGITUD,
                      posJ.y + dirY * GANCHO_LONGITUD};

    RaycastCtx ctx; ctx.gancheables = gancheables;
    b2World_CastRay(worldId, origen, destino, b2DefaultQueryFilter(),
                    raycastCallback, &ctx);

    if (!ctx.golpeo) return false;

    anclarEn({ctx.puntoContacto.x, ctx.puntoContacto.y});
    return true;
}

// ── soltar ───────────────────────────────────────────────────
void Gancho::soltar()
{
    destruirCuerda();
    if (anclajeValido) { b2DestroyBody(bodyAnclaje); anclajeValido = false; }
    estado = EstadoGancho::Inactivo;
}

// ── update ───────────────────────────────────────────────────
void Gancho::update(float dt)
{
    if (estado == EstadoGancho::Inactivo) return;

    // ── Retracción activa: impulso hacia el punto de anclaje ──────────────
    // Jala al jugador hacia el techo progresivamente.
    // El DistanceJoint actúa como tope de longitud máxima (péndulo),
    // el impulso hace que el jugador suba activamente.
    if (longitudActual > GANCHO_LONG_MIN) {
        b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
        float  dx   = puntoAnclaje.x - posJ.x;
        float  dy   = puntoAnclaje.y - posJ.y;
        float  dist = std::sqrt(dx * dx + dy * dy);

        if (dist > GANCHO_LONG_MIN) {
            float inv = 1.f / dist;
            b2Vec2 impulso = {
                dx * inv * GANCHO_RETRACCION * dt,
                dy * inv * GANCHO_RETRACCION * dt
            };
            b2Body_ApplyLinearImpulseToCenter(cuerpoJugador, impulso, true);
        }

        longitudActual -= GANCHO_RETRACCION * dt;
        if (longitudActual < GANCHO_LONG_MIN)
            longitudActual = GANCHO_LONG_MIN;

        // Acortar el joint para que la cuerda visual también se recoja
        if (jointValido && b2Joint_IsValid(joint))
            b2DistanceJoint_SetLengthRange(joint,
                                           GANCHO_LONG_MIN * 0.5f,
                                           longitudActual);
    }

    // ── Visual ───────────────────────────────────────────────────────────
    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    lineaCuerda[0].position = {posJ.x, posJ.y};
    lineaCuerda[0].color    = sf::Color(220, 180, 30, 220);
    lineaCuerda[1].position = puntoAnclaje;
    lineaCuerda[1].color    = sf::Color(255, 230, 80, 200);
    punta.setPosition(puntoAnclaje);
}

// ── draw ─────────────────────────────────────────────────────
void Gancho::draw(sf::RenderWindow& v, sf::Color col)
{
    if (estado == EstadoGancho::Inactivo) return;
    lineaCuerda[0].color = col;
    lineaCuerda[1].color = sf::Color(col.r, col.g, col.b, 160);
    v.draw(lineaCuerda);
    v.draw(punta);
}

bool         Gancho::estaActivo()      const { return estado == EstadoGancho::Anclado; }
sf::Vector2f Gancho::getPuntoAnclaje() const { return puntoAnclaje; }

// ── anclarEn ─────────────────────────────────────────────────
void Gancho::anclarEn(sf::Vector2f punto)
{
    puntoAnclaje   = punto;
    estado         = EstadoGancho::Anclado;

    // Cuerpo estático en el punto de anclaje
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = {punto.x, punto.y};
    bodyAnclaje   = b2CreateBody(worldId, &bd);
    anclajeValido = true;

    // Distancia inicial
    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float  dx   = punto.x - posJ.x;
    float  dy   = punto.y - posJ.y;
    float  dist = std::sqrt(dx * dx + dy * dy);
    longitudActual = dist;

    // Joint de distancia (péndulo elástico)
    b2DistanceJointDef jd = b2DefaultDistanceJointDef();
    jd.bodyIdA      = cuerpoJugador;
    jd.bodyIdB      = bodyAnclaje;
    jd.localAnchorA = {0.f, 0.f};
    jd.localAnchorB = {0.f, 0.f};
    jd.length       = dist;
    jd.minLength    = GANCHO_LONG_MIN * 0.5f;
    jd.maxLength    = dist;
    jd.hertz        = GANCHO_FREQ;
    jd.dampingRatio = GANCHO_DAMP;
    jd.enableSpring = true;
    jd.enableLimit  = true;

    joint      = b2CreateDistanceJoint(worldId, &jd);
    jointValido = true;
}

// ── destruirCuerda ───────────────────────────────────────────
void Gancho::destruirCuerda()
{
    if (jointValido) {
        if (b2Joint_IsValid(joint)) b2DestroyJoint(joint);
        jointValido = false;
    }
}
