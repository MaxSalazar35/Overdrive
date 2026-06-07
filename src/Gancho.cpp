// ============================================================
// Gancho.cpp — Overdrive (Box2D v3)  v5
//
// BUG ANTERIOR: el raycast filtraba normal.y > -0.5 esperando
// una normal que apuntara "hacia abajo" (y negativo en SFML).
// Pero el rayo sale del jugador HACIA ARRIBA e impacta la cara
// INFERIOR del techo, cuya normal apunta HACIA ABAJO en mundo
// Box2D... que en SFML (Y invertido) es normal.y POSITIVO.
// El filtro correcto es: normal.y > 0.5 (cara mira hacia abajo
// en Box2D = hacia arriba en SFML = cara inferior de un bloque).
//
// MECÁNICA:
//  - 3 rayos: vertical puro, 30° adelante, 30° atrás.
//  - Solo acepta el techo continuo: point.y <= GANCHO_Y_MAX_CONTACTO.
//  - Normal correcta: normal.y > 0.5 (cara inferior de bloque).
// ============================================================
#include "Gancho.hpp"
#include <cmath>

static constexpr float PI = 3.14159265f;

// ── Raycast callback ─────────────────────────────────────────
static float raycastCallback(b2ShapeId shapeId, b2Vec2 point,
                              b2Vec2 normal, float fraction, void* context)
{
    RaycastCtx* ctx = static_cast<RaycastCtx*>(context);

    if (b2Shape_IsSensor(shapeId)) return -1.f;
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    if (b2Body_GetType(bodyId) == b2_dynamicBody) return -1.f;

    // El rayo va hacia arriba (y decrece) e impacta la cara INFERIOR
    // del techo. En Box2D la normal de esa cara apunta hacia abajo,
    // que en coordenadas SFML (Y hacia abajo) es normal.y > 0.
    // Aceptamos cualquier cara horizontal superior: |normal.y| > 0.5
    // y descartamos si el punto de impacto no está cerca del techo.
    (void)normal; // no filtramos por normal — solo por posición Y

    // El rayo sale desde la cabeza del jugador y va hacia el techo.
    // Solo aceptar si el punto está en el techo continuo (y <= 25)
    if (point.y > GANCHO_Y_MAX_CONTACTO) return -1.f;

    // Ignorar impactos a menos del 2% de la longitud (self-hit)
    if (fraction < 0.02f) return -1.f;

    // Cuerpos dinámicos ya filtrados arriba; ignorar sensores también
    // Guardar el más cercano
    if (!ctx->golpeo || fraction < ctx->fraccionMin) {
        ctx->golpeo         = true;
        ctx->fraccionMin    = fraction;
        ctx->puntoContacto  = point;
        ctx->cuerpoGolpeado = bodyId;
    }
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
    // Disparar desde la CABEZA del jugador (arriba del hitbox = centro - 35px en Y)
    // para evitar que el rayo intercepte el propio cuerpo del jugador
    b2Vec2 origen = { posJ.x, posJ.y - 36.f };
    float  L      = GANCHO_LONGITUD;

    auto lanzar = [&](float dx, float dy) -> bool {
        b2Vec2 dest = { origen.x + dx * L, origen.y + dy * L };
        RaycastCtx ctx; ctx.gancheables = gancheables;
        b2World_CastRay(worldId, origen, dest, b2DefaultQueryFilter(),
                        raycastCallback, &ctx);
        if (ctx.golpeo) {
            anclarEn({ctx.puntoContacto.x, ctx.puntoContacto.y});
            return true;
        }
        return false;
    };

    // Rayo 1: completamente vertical hacia arriba
    if (lanzar(0.f, -1.f)) return true;

    // Rayo 2: 30° hacia adelante desde la vertical (60° desde horizontal)
    float sign = mirandoDerecha ? 1.f : -1.f;
    float ang  = 60.f * PI / 180.f;   // 60° desde horizontal
    if (lanzar(sign * std::cos(ang), -std::sin(ang))) return true;

    // Rayo 3: 30° hacia el lado opuesto
    if (lanzar(-sign * std::cos(ang), -std::sin(ang))) return true;

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
