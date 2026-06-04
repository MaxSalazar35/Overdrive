// ============================================================
// Gancho.cpp — Overdrive (Box2D v3)
// Comportamiento original: al anclar aplica un impulso fuerte
// hacia el punto de anclaje, luego el joint de distancia
// controla el péndulo. Solo en superficies marcadas gancheables.
// ============================================================
#include "Gancho.hpp"
#include <cmath>

static float raycastCallback(b2ShapeId shapeId, b2Vec2 point,
                              b2Vec2 /*normal*/, float fraction, void* context)
{
    RaycastCtx* ctx = static_cast<RaycastCtx*>(context);
    if (b2Shape_IsSensor(shapeId)) return -1.f;
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_GetType(bodyId) == b2_dynamicBody) return -1.f;

    if (ctx->gancheables) {
        bool ok = false;
        for (const auto& bid : *ctx->gancheables)
            if (bid.index1 == bodyId.index1 && bid.revision == bodyId.revision)
                { ok = true; break; }
        if (!ok) return -1.f;
    }
    ctx->golpeo         = true;
    ctx->puntoContacto  = point;
    ctx->cuerpoGolpeado = bodyId;
    return fraction;
}

Gancho::Gancho(b2WorldId worldId, b2BodyId cuerpoJugador)
    : worldId(worldId), cuerpoJugador(cuerpoJugador)
{
    punta.setRadius(5.f); punta.setOrigin(5.f, 5.f);
    punta.setFillColor(sf::Color(255, 220, 80));
    punta.setOutlineColor(sf::Color(200, 160, 20));
    punta.setOutlineThickness(1.5f);
    lineaCuerda.setPrimitiveType(sf::Lines);
    lineaCuerda.resize(2);
}

Gancho::~Gancho() { destruirCuerda(); }

bool Gancho::disparar(sf::Vector2f dir)
{
    if (estado == EstadoGancho::Anclado) { soltar(); return false; }

    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float  mag  = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (mag < 0.001f) return false;
    dir /= mag;

    b2Vec2 origen  = posJ;
    b2Vec2 destino = {posJ.x + dir.x * GANCHO_LONGITUD,
                      posJ.y + dir.y * GANCHO_LONGITUD};

    RaycastCtx ctx; ctx.gancheables = gancheables;
    b2World_CastRay(worldId, origen, destino, b2DefaultQueryFilter(),
                    raycastCallback, &ctx);
    if (!ctx.golpeo) return false;

    anclarEn({ctx.puntoContacto.x, ctx.puntoContacto.y});
    return true;
}

void Gancho::soltar()
{
    destruirCuerda();
    if (anclajeValido) { b2DestroyBody(bodyAnclaje); anclajeValido = false; }
    estado = EstadoGancho::Inactivo;
}

void Gancho::update(float /*dt*/)
{
    if (estado == EstadoGancho::Inactivo) return;
    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    lineaCuerda[0].position = {posJ.x, posJ.y};
    lineaCuerda[0].color    = sf::Color(200,160,20,200);
    lineaCuerda[1].position = puntoAnclaje;
    lineaCuerda[1].color    = sf::Color(200,160,20,200);
    punta.setPosition(puntoAnclaje);
}

void Gancho::draw(sf::RenderWindow& v, sf::Color col)
{
    if (estado == EstadoGancho::Inactivo) return;
    lineaCuerda[0].color = col;
    lineaCuerda[1].color = sf::Color(col.r, col.g, col.b, 150);
    v.draw(lineaCuerda); v.draw(punta);
}

bool         Gancho::estaActivo()      const { return estado == EstadoGancho::Anclado; }
sf::Vector2f Gancho::getPuntoAnclaje() const { return puntoAnclaje; }

void Gancho::anclarEn(sf::Vector2f punto)
{
    puntoAnclaje = punto;
    estado       = EstadoGancho::Anclado;

    // Cuerpo estático de anclaje
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type = b2_staticBody; bd.position = {punto.x, punto.y};
    bodyAnclaje = b2CreateBody(worldId, &bd);
    anclajeValido = true;

    // ── Impulso hacia el punto de anclaje (comportamiento original) ──
    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float  dx   = punto.x - posJ.x;
    float  dy   = punto.y - posJ.y;
    float  dist = std::sqrt(dx*dx + dy*dy);
    if (dist > 0.f) {
        b2Vec2 dir = {dx / dist, dy / dist};
        // Impulso: GANCHO_IMPULSO en dirección al punto
        // Multiplicamos por masa (estimada 1.0 = bodyDef density*area)
        b2Body_ApplyLinearImpulseToCenter(
            cuerpoJugador,
            {dir.x * GANCHO_IMPULSO, dir.y * GANCHO_IMPULSO},
            true);
    }

    // ── Joint de distancia para el péndulo ──────────────────
    b2DistanceJointDef jd = b2DefaultDistanceJointDef();
    jd.bodyIdA      = cuerpoJugador;
    jd.bodyIdB      = bodyAnclaje;
    jd.localAnchorA = {0.f, 0.f};
    jd.localAnchorB = {0.f, 0.f};
    jd.length       = dist;
    jd.minLength    = 20.f;
    jd.maxLength    = dist;
    jd.hertz        = GANCHO_FREQ;
    jd.dampingRatio = GANCHO_DAMP;
    jd.enableSpring = true;
    jd.enableLimit  = true;

    joint = b2CreateDistanceJoint(worldId, &jd);
    jointValido = true;
}

void Gancho::destruirCuerda()
{
    if (jointValido) {
        if (b2Joint_IsValid(joint)) b2DestroyJoint(joint);
        jointValido = false;
    }
}
