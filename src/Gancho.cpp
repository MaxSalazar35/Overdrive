// ============================================================
// Gancho.cpp — Overdrive (Box2D v3)
// ============================================================
#include "Gancho.hpp"
#include <cmath>
#include <iostream>

// Callback de raycast para Box2D v3
static float raycastCallback(b2ShapeId shapeId, b2Vec2 point,
                              b2Vec2 normal, float fraction, void* context)
{
    RaycastCtx* ctx = static_cast<RaycastCtx*>(context);

    // Ignorar sensores
    if (b2Shape_IsSensor(shapeId)) return -1.f;

    // Ignorar cuerpos dinámicos (solo anclar en estáticos/kinematic)
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_GetType(bodyId) == b2_dynamicBody) return -1.f;

    ctx->golpeo         = true;
    ctx->puntoContacto  = point;
    ctx->cuerpoGolpeado = bodyId;
    return fraction; // seguir hasta el más cercano
}

Gancho::Gancho(b2WorldId worldId, b2BodyId cuerpoJugador)
    : worldId(worldId), cuerpoJugador(cuerpoJugador)
{
    punta.setRadius(5.f);
    punta.setOrigin(5.f, 5.f);
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

    float mag = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (mag < 0.001f) return false;
    dir /= mag;

    b2Vec2 origen  = posJ;
    b2Vec2 destino = {posJ.x + dir.x * GANCHO_LONGITUD,
                      posJ.y + dir.y * GANCHO_LONGITUD};

    RaycastCtx ctx;
    b2QueryFilter filtro = b2DefaultQueryFilter();
    b2World_CastRay(worldId, origen, destino, filtro, raycastCallback, &ctx);

    if (!ctx.golpeo) return false;

    anclarEn({ctx.puntoContacto.x, ctx.puntoContacto.y});
    return true;
}

void Gancho::soltar()
{
    destruirCuerda();
    if (anclajeValido) {
        b2DestroyBody(bodyAnclaje);
        anclajeValido = false;
    }
    estado = EstadoGancho::Inactivo;
}

void Gancho::update(float /*dt*/)
{
    if (estado == EstadoGancho::Inactivo) return;

    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);

    // Acortar la cuerda para jalar al jugador
    if (cuerdaValida) {
        float longActual = b2DistanceJoint_GetLength(cuerda);
        if (longActual > 30.f)
            b2DistanceJoint_SetLength(cuerda, longActual * 0.985f);
    }

    // Visual
    lineaCuerda[0].position = {posJ.x, posJ.y};
    lineaCuerda[0].color    = sf::Color(180, 180, 180, 220);
    lineaCuerda[1].position = puntoAnclaje;
    lineaCuerda[1].color    = sf::Color(140, 140, 140, 180);
    punta.setPosition(puntoAnclaje);
}

void Gancho::draw(sf::RenderWindow& ventana, sf::Color colorCuerda)
{
    if (estado == EstadoGancho::Inactivo) return;
    lineaCuerda[0].color = colorCuerda;
    ventana.draw(lineaCuerda);
    ventana.draw(punta);
}

void Gancho::anclarEn(sf::Vector2f punto)
{
    puntoAnclaje = punto;
    estado       = EstadoGancho::Anclado;

    // Cuerpo estático en el punto de anclaje
    b2BodyDef bd    = b2DefaultBodyDef();
    bd.type         = b2_staticBody;
    bd.position     = {punto.x, punto.y};
    bodyAnclaje     = b2CreateBody(worldId, &bd);
    anclajeValido   = true;

    // Distancia actual
    b2Vec2 posJ = b2Body_GetPosition(cuerpoJugador);
    float dx    = punto.x - posJ.x;
    float dy    = punto.y - posJ.y;
    float dist  = std::sqrt(dx*dx + dy*dy);

    // Crear joint de distancia (Box2D v3)
    b2DistanceJointDef jd = b2DefaultDistanceJointDef();
    jd.bodyIdA            = cuerpoJugador;
    jd.bodyIdB            = bodyAnclaje;
    jd.localAnchorA       = {0.f, 0.f};
    jd.localAnchorB       = {0.f, 0.f};
    jd.length             = dist;
    jd.enableSpring       = true;
    jd.hertz              = GANCHO_FREQ;
    jd.dampingRatio       = GANCHO_DAMP;
    jd.enableLimit        = true;
    jd.minLength          = 0.f;
    jd.maxLength          = dist;
    jd.collideConnected   = false;

    cuerda       = b2CreateDistanceJoint(worldId, &jd);
    cuerdaValida = true;

    // Impulso inicial hacia el anclaje
    float mag = std::sqrt(dx*dx + dy*dy);
    if (mag > 0.1f) {
        b2Vec2 imp = {(dx/mag) * GANCHO_IMPULSO, (dy/mag) * GANCHO_IMPULSO};
        b2Body_ApplyLinearImpulseToCenter(cuerpoJugador, imp, true);
    }
}

void Gancho::destruirCuerda()
{
    if (cuerdaValida) {
        b2DestroyJoint(cuerda);
        cuerdaValida = false;
    }
}
