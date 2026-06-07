#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constantes.hpp"

// ============================================================
// Camara.hpp — Overdrive
// Sigue al jugador más adelante con lerp.
// Jugador que sale por la izquierda → eliminado.
// Efecto de screen shake al respawnear.
// ============================================================

class Camara {
public:
    Camara(float anchoMundo, float altoMundo);

    void update(const std::vector<sf::Vector2f>& posiciones, float dt);
    void aplicar(sf::RenderWindow& ventana);
    void restaurarVista(sf::RenderWindow& ventana);

    void resetearAlInicio() {
        vista.setCenter(ANCHO_VENTANA / 2.f, altoMundo / 2.f);
    }

    // Activa un shake de cámara (duracion en segundos, intensidad en px)
    void activarShake(float duracion = 0.45f, float intensidad = 12.f) {
        shakeDuracion  = duracion;
        shakeTimer     = duracion;
        shakeIntensidad = intensidad;
    }

    bool         fueraDePantalla(sf::Vector2f pos) const;
    sf::FloatRect getVistaMundo()                   const;
    sf::Vector2f  getCentro()                       const { return vista.getCenter(); }

private:
    sf::View vista;
    float    anchoMundo, altoMundo;
    float    xObjetivo;

    // Shake
    float shakeDuracion   = 0.f;
    float shakeTimer      = 0.f;
    float shakeIntensidad = 0.f;

    int idLider(const std::vector<sf::Vector2f>& pos) const;
};
