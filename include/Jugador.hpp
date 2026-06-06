#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <memory>
#include "Constantes.hpp"
#include "Animacion.hpp"
#include "Gancho.hpp"
#include "Input.hpp"
#include "Particulas.hpp"

// ============================================================
// Jugador.hpp — Overdrive (Box2D v3)
// ============================================================

enum class EstadoJug { Idle, Corriendo, Saltando, Cayendo, Deslizando, Muerto };
enum AnimID { ANIM_IDLE=0, ANIM_CORRER, ANIM_SALTAR, ANIM_DESLIZAR };

class Jugador {
public:
    Jugador(b2WorldId worldId, sf::Vector2f posInicial,
            int idJugador, SistemaParticulas& particulas);
    ~Jugador();

    void procesarEntrada(float dt);
    void update(float dt);
    void draw(sf::RenderWindow& ventana);

    sf::Vector2f  getPosicion()    const;
    float         getVelocidadX()  const;
    sf::FloatRect getBounds()      const;
    int           getID()          const { return id; }
    bool          estaMuerto()     const { return estado == EstadoJug::Muerto; }
    bool          estaEnSuelo()    const { return enSuelo; }

    void eliminar();
    void resetear(sf::Vector2f pos);
    void recibirImpacto();
    // Frena la velocidad X por un factor (0=para, 1=sin cambio)
    void aplicarFreno(float factor);

    Gancho* gancho = nullptr;

private:
    int        id;
    EstadoJug  estado;
    EstadoJug  estadoAnterior;   // para detectar transiciones
    bool       enSuelo;
    bool       mirandoDerecha;
    float      tiempoSinSuelo;
    bool       ganchoPresionado;
    bool       deslizPresionado;
    float      timerDesliz;

    // ── Animacion de salida del desliz ───────────────────────
    // Cuando se suelta el botón, congelamos el último frame un instante
    float timerSalidaDesliz;
    static constexpr float DURACION_SALIDA_DESLIZ = 0.12f; // segundos

    // ── Estado del salto para animar correctamente ────────────
    // Mientras subimos (vy < 0) mostramos la anim de salto hacia adelante.
    // Cuando empezamos a bajar (vy >= 0) congelamos el último frame.
    bool animCaidaCongelada;     // true = ya mostramos el frame de caida

    // ── Salto por fuerza sostenida (estilo Mario) ─────────────
    bool  saltando;
    bool  puedesSaltar;
    float timerSalto;
    float cooldownSalto;

    // ── Desliz en bajada ──────────────────────────────────────
    // Si la velocidad Y del jugador es positiva (baja) al iniciar el desliz,
    // se aplica un impulso extra una sola vez.
    bool  deslizBoostAplicado;

    b2WorldId  worldId;
    b2BodyId   cuerpo;

    sf::Texture textura;
    sf::Sprite  sprite;
    Animacion   anim;
    InputHandler input;
    SistemaParticulas& particulas;

    sf::Clock relojEstela;

    void actualizarEstado();
    void actualizarAnimacion();
};
