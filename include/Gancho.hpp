#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>
#include "Constantes.hpp"

// ============================================================
// Gancho.hpp — Overdrive (Box2D v3)
//
// MECÁNICA:
//  - Disparo diagonal a 70° hacia arriba-adelante según dirección del jugador.
//    Solo se ancla en el techo continuo (y <= GANCHO_Y_MAX_CONTACTO).
//    Sin límite horizontal — el ángulo y la longitud del rayo lo delimitan.
//
//  - Retráctil: al anclar, aplica impulso continuo hacia el punto de anclaje
//    (GANCHO_RETRACCION px/s). El joint de distancia controla el péndulo.
//
//  - Soltar: el jugador sale disparado con la velocidad acumulada.
// ============================================================

enum class EstadoGancho { Inactivo, Anclado };

struct RaycastCtx {
    bool         golpeo        = false;
    b2Vec2       puntoContacto = {0.f, 0.f};
    b2BodyId     cuerpoGolpeado = {};
    const std::vector<b2BodyId>* gancheables = nullptr;
};

class Gancho {
public:
    Gancho(b2WorldId worldId, b2BodyId cuerpoJugador);
    ~Gancho();

    void setGancheables(const std::vector<b2BodyId>* lista) { gancheables = lista; }

    // dir: dirección normalizada del disparo.
    // mirandoDerecha: para calcular límite horizontal.
    bool disparar(sf::Vector2f dir, bool mirandoDerecha);
    void soltar();
    void update(float dt);
    void draw(sf::RenderWindow& ventana, sf::Color colorCuerda);

    bool         estaActivo()      const;
    sf::Vector2f getPuntoAnclaje() const;

private:
    b2WorldId    worldId;
    b2BodyId     cuerpoJugador;
    b2BodyId     bodyAnclaje   = {};
    b2JointId    joint         = {};
    bool         jointValido   = false;
    bool         anclajeValido = false;

    EstadoGancho estado       = EstadoGancho::Inactivo;
    sf::Vector2f puntoAnclaje = {0.f, 0.f};
    float        longitudActual = 0.f;   // longitud corriente del joint (se reduce)

    sf::CircleShape punta;
    sf::VertexArray lineaCuerda;

    const std::vector<b2BodyId>* gancheables = nullptr;

    void anclarEn(sf::Vector2f punto);
    void destruirCuerda();
};
