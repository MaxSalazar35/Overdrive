#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constantes.hpp"

class HUD {
public:
    HUD(sf::RenderWindow& ventana);

    void update(
        const std::vector<float>& velocidades,
        const std::vector<bool>&  ganchosActivos,
        const std::vector<int>&   vidas,
        int idLider,
        float dt
    );

    void draw(sf::RenderWindow& ventana);

    void mostrarSobrevivio(int idSobreviviente);
    void ocultarVictoria();
    bool estaAnimandoVictoria() const { return animVictoria; }
    void mostrarFinJuego(int idGanador);

private:
    sf::Font fuente;
    bool fuenteCargada = false;

    int numJugadores = 2;
    float anchoV, altoV;

    struct Panel {
        sf::RectangleShape fondo;
        sf::RectangleShape borde;

        // Nombre del jugador (nuevo)
        sf::Text           textoNombre;

        // Velocímetro
        sf::RectangleShape barraFondo;
        sf::RectangleShape barraVel;
        sf::Text           textoVel;

        // Vidas
        std::vector<sf::CircleShape> circulosVida;

        // Gancho
        sf::CircleShape  iconoGancho;
        sf::Text         textoGancho;

        // Líder
        sf::Text corona;
    };
    std::vector<Panel> paneles;

    bool          animVictoria  = false;
    bool          finJuego      = false;
    float         timerVictoria = 0.f;
    float         escalaVict    = 0.f;
    sf::RectangleShape fondoOsc;
    sf::Text           textoVict;
    sf::Text           textoSubVict;

    sf::Texture texGanador;
    sf::Sprite  sprGanador;
    bool        texGanadorCargada = false;

    sf::Color colorJugador(int id) const;
    void      inicializarPaneles(const std::vector<int>& vidasIniciales);
};
