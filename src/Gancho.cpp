// ============================================================
// Gancho.cpp — Overdrive (Box2D v3)  v4
//
// MECÁNICA:
//  - Disparo siempre hacia el techo. Estrategia de 3 rayos:
//      1. Recto hacia arriba (90°) — alcance directo
//      2. 30° a la derecha del vertical (hacia adelante)
//      3. 30° a la izquierda del vertical (hacia atrás)
//  - Solo se ancla en el techo continuo: point.y <= GANCHO_Y_MAX_CONTACTO
//    y normal.y < -0.5 (cara mirando hacia abajo).
//  - Al anclar: DistanceJoint como péndulo + impulso retráctil.
// ============================================================
#include "Gancho.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

// ── Raycast callback — SOLO techo (y <= GANCHO_Y_MAX_CONTACTO) ─────────
static float raycastCallback(b2ShapeId shapeId, b2Vec2 point,
                              b2Vec2 normal, float fraction, void* context)
{
    RaycastCtx* ctx = static_cast<RaycastCtx*>(context);

    if (b2Shape_IsSensor(shapeId)) return -1.f;
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_GetType(bodyId) == b2_dynamicBody) return -1.f;

    // Cara que mira hacia abajo (normal apunta hacia abajo, y negativo = arriba en SFML)
    if (normal.y > -0.5f) return -1.f;

    // Solo el techo continuo (y = 0..GANCHO_Y_MAX_CONTACTO)
    if (point.y > GANCHO_Y_MAX_CONTACTO) return -1.f;

    // Guardar el impacto más cercano
    if (!ctx->golpeo || fraction < ctx->fraccionMin) {
        ctx->golpeo         = true;
        ctx->fraccionMin    = fraction;
        ctx->puntoContacto  = point;
        ctx->cuerpoGolpeado = bodyId;
    }
    // Devolver fraction para que el raycast siga buscando algo más cercano
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
bool Gancho::disparar(sf::Vector2f /*dir*/, bool mirandoDerecha)
{
    if (estado == EstadoGancho::Anclado) { soltar(); return false; }

    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float  L    = GANCHO_LONGITUD;

    // Helper: lanza un rayo con una dirección (dx,dy) normalizada
    auto lanzar = [&](float dx, float dy) -> bool {
        b2Vec2 dest = { posJ.x + dx * L, posJ.y + dy * L };
        RaycastCtx ctx; ctx.gancheables = gancheables;
        b2World_CastRay(worldId, posJ, dest, b2DefaultQueryFilter(),
                        raycastCallback, &ctx);
        if (ctx.golpeo) {
            anclarEn({ctx.puntoContacto.x, ctx.puntoContacto.y});
            return true;
        }
        return false;
    };

    // Rayo 1: recto hacia arriba (90° desde horizontal)
    if (lanzar(0.f, -1.f)) return true;

    // Rayo 2: 30° a la derecha del vertical (60° desde horizontal)
    //   sin(60°) ≈ 0.866 (componente vertical), cos(60°) = 0.5 (horizontal)
    float ang60 = 60.f * PI / 180.f;
    float sign  = mirandoDerecha ? 1.f : -1.f;
    if (lanzar(sign * std::cos(ang60), -std::sin(ang60))) return true;

    // Rayo 3: 30° al otro lado (opuesto a la dirección de marcha)
    if (lanzar(-sign * std::cos(ang60), -std::sin(ang60))) return true;

    return false;
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

    // Retracción: impulso hacia el punto de anclaje
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

        if (jointValido && b2Joint_IsValid(joint))
            b2DistanceJoint_SetLengthRange(joint,
                                           GANCHO_LONG_MIN * 0.5f,
                                           longitudActual);
    }

    // Visual
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
    puntoAnclaje  = punto;
    estado        = EstadoGancho::Anclado;

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = {punto.x, punto.y};
    bodyAnclaje  = b2CreateBody(worldId, &bd);
    anclajeValido = true;

    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float  dx   = punto.x - posJ.x;
    float  dy   = punto.y - posJ.y;
    float  dist = std::sqrt(dx * dx + dy * dy);
    longitudActual = dist;

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

    joint       = b2CreateDistanceJoint(worldId, &jd);
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
