#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constantes.hpp"

// ============================================================
// Camara.hpp — Overdrive
// Sigue al jugador más adelante con lerp.
// Jugador que sale por la izquierda → eliminado.
// ============================================================

class Camara {
public:
    Camara(float anchoMundo, float altoMundo);

    void update(const std::vector<sf::Vector2f>& posiciones, float dt);
    void aplicar(sf::RenderWindow& ventana);
    void restaurarVista(sf::RenderWindow& ventana);  // para HUD

    // Resetea la cámara al centro del spawn (llamar al respawnear)
    void resetearAlInicio() {
        vista.setCenter(ANCHO_VENTANA / 2.f, altoMundo / 2.f);
    }

    bool         fueraDePantalla(sf::Vector2f pos) const;
    sf::FloatRect getVistaMundo()                   const;
    sf::Vector2f  getCentro()                       const { return vista.getCenter(); }

private:
    sf::View vista;
    float    anchoMundo, altoMundo;
    float    xObjetivo;

    int idLider(const std::vector<sf::Vector2f>& pos) const;
};
