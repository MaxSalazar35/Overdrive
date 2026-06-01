// ============================================================
// HUD.cpp — Overdrive
// ============================================================
#include "HUD.hpp"
#include <cmath>
#include <string>

static const int VIDAS_MAX = 3;

HUD::HUD(sf::RenderWindow& ventana)
{
    anchoV = (float)ventana.getSize().x;
    altoV  = (float)ventana.getSize().y;

    fuenteCargada = fuente.loadFromFile("assets/fonts/Ring.ttf");

    std::vector<int> vidasInic(2, VIDAS_MAX);
    inicializarPaneles(vidasInic);

    // ── Fondo oscuro de victoria ──────────────────────────────
    fondoOsc.setSize({anchoV, altoV});
    fondoOsc.setFillColor(sf::Color(0, 0, 0, 0));

    textoVict.setFont(fuente);
    textoVict.setCharacterSize(80);
    textoVict.setOutlineThickness(4.f);
    textoVict.setOutlineColor(sf::Color::Black);

    textoSubVict.setFont(fuente);
    textoSubVict.setCharacterSize(32);
    textoSubVict.setFillColor(sf::Color(200, 200, 200));
    textoSubVict.setString("Presiona A / Enter para continuar");

    // Sprite del ganador (se configura en mostrarFinJuego)
    sprGanador.setScale(2.5f, 2.5f);
}

void HUD::inicializarPaneles(const std::vector<int>& vidasIniciales)
{
    paneles.clear();
    for (int i = 0; i < numJugadores; ++i) {
        Panel p;
        float xBase = (i == 0) ? 12.f : anchoV - 212.f;
        float yBase = 12.f;
        sf::Color cj = colorJugador(i);

        p.fondo.setSize({200.f, 100.f});
        p.fondo.setPosition(xBase, yBase);
        p.fondo.setFillColor(sf::Color(0, 0, 0, 150));

        p.borde.setSize({200.f, 100.f});
        p.borde.setPosition(xBase, yBase);
        p.borde.setFillColor(sf::Color::Transparent);
        p.borde.setOutlineColor(cj);
        p.borde.setOutlineThickness(2.f);

        // Barra de velocidad
        p.barraFondo.setSize({190.f, 12.f});
        p.barraFondo.setPosition(xBase + 5.f, yBase + 80.f);
        p.barraFondo.setFillColor(sf::Color(40, 40, 40));

        p.barraVel.setSize({0.f, 12.f});
        p.barraVel.setPosition(xBase + 5.f, yBase + 80.f);
        p.barraVel.setFillColor(cj);

        // Texto velocidad
        p.textoVel.setFont(fuente);
        p.textoVel.setCharacterSize(20);
        p.textoVel.setFillColor(sf::Color::White);
        p.textoVel.setPosition(xBase + 8.f, yBase + 8.f);
        p.textoVel.setString("J" + std::to_string(i+1) + "  0 km/h");

        // Indicador gancho
        p.iconoGancho.setRadius(7.f);
        p.iconoGancho.setOrigin(7.f, 7.f);
        p.iconoGancho.setPosition(xBase + 170.f, yBase + 50.f);
        p.iconoGancho.setFillColor(sf::Color(60,60,60));
        p.iconoGancho.setOutlineColor(sf::Color(255,220,80));
        p.iconoGancho.setOutlineThickness(2.f);

        p.textoGancho.setFont(fuente);
        p.textoGancho.setCharacterSize(11);
        p.textoGancho.setFillColor(sf::Color(200,200,200));
        p.textoGancho.setPosition(xBase + 8.f, yBase + 58.f);
        p.textoGancho.setString("GANCHO");

        // Corona de líder
        p.corona.setFont(fuente);
        p.corona.setCharacterSize(22);
        p.corona.setFillColor(sf::Color(255, 215, 0));
        p.corona.setString("LIDER");
        p.corona.setPosition(xBase + 8.f, yBase + 30.f);

        // Corazones de vida (círculos)
        int vMax = (i < (int)vidasIniciales.size()) ? vidasIniciales[i] : VIDAS_MAX;
        for (int r = 0; r < vMax; ++r) {
            sf::CircleShape c(7.f);
            c.setOrigin(7.f, 7.f);
            c.setPosition(xBase + 12.f + r * 20.f, yBase + 62.f);
            c.setFillColor(cj);
            c.setOutlineColor(sf::Color::White);
            c.setOutlineThickness(1.5f);
            p.circulosVida.push_back(c);
        }

        paneles.push_back(p);
    }
}

void HUD::update(
    const std::vector<float>& velocidades,
    const std::vector<bool>&  ganchosActivos,
    const std::vector<int>&   vidas,
    int /*idLider*/, float dt)
{
    const float VEL_MAX_VIS = JUG_VEL_MAX;

    for (int i = 0; i < numJugadores && i < (int)paneles.size(); ++i) {
        float velAbs  = std::abs(velocidades[i]);
        float velKmh  = velAbs * 0.36f;
        float pct     = std::min(velAbs / VEL_MAX_VIS, 1.f);

        paneles[i].textoVel.setString(
            "J" + std::to_string(i+1) + "  " +
            std::to_string((int)velKmh) + " km/h"
        );
        paneles[i].barraVel.setSize({pct * 190.f, 12.f});
        paneles[i].barraVel.setFillColor(
            pct > 0.85f ? sf::Color(255, 80, 80) : colorJugador(i)
        );

        // Gancho
        paneles[i].iconoGancho.setFillColor(
            ganchosActivos[i] ? sf::Color(255,220,80) : sf::Color(40,40,40)
        );

        // Vidas: actualizar color de círculos
        int v = (i < (int)vidas.size()) ? vidas[i] : 0;
        for (int r = 0; r < (int)paneles[i].circulosVida.size(); ++r) {
            paneles[i].circulosVida[r].setFillColor(
                r < v ? colorJugador(i) : sf::Color(30, 30, 30)
            );
        }

        // Mostrar LIDER solo al que va adelante
        // (se pinta condicionalmente en draw)
    }

    // Animación de victoria (pop scale)
    if (animVictoria) {
        timerVictoria += dt;
        sf::Uint8 alfa = (sf::Uint8)std::min(timerVictoria * 200.f, 180.f);
        fondoOsc.setFillColor(sf::Color(0, 0, 0, alfa));
        escalaVict = std::min(1.f, timerVictoria * 4.f);
        float s = escalaVict < 0.8f
            ? escalaVict / 0.8f * 1.2f
            : 1.2f - (escalaVict - 0.8f) / 0.2f * 0.2f;
        textoVict.setScale(s, s);

        sf::FloatRect b = textoVict.getLocalBounds();
        textoVict.setOrigin(b.width/2.f, b.height/2.f);

        if (finJuego) {
            // Con imagen: texto arriba, imagen en centro, subtexto abajo
            textoVict.setPosition(anchoV/2.f, altoV * 0.18f);

            if (texGanadorCargada) {
                sf::FloatRect sb = sprGanador.getLocalBounds();
                sprGanador.setOrigin(sb.width/2.f, sb.height/2.f);
                sprGanador.setPosition(anchoV/2.f, altoV * 0.50f);
                float sc = std::min(s * 1.1f, 1.1f);
                sprGanador.setScale(sc * 2.5f, sc * 2.5f);
            }
        } else {
            textoVict.setPosition(anchoV/2.f, altoV * 0.42f);
        }

        sf::FloatRect b2 = textoSubVict.getLocalBounds();
        textoSubVict.setOrigin(b2.width/2.f, b2.height/2.f);
        textoSubVict.setPosition(anchoV/2.f, altoV * 0.80f);
        sf::Uint8 alfaSub = (sf::Uint8)(std::sin(timerVictoria * 4.f) * 80.f + 160.f);
        textoSubVict.setFillColor(sf::Color(200,200,200,alfaSub));
    }
}

void HUD::draw(sf::RenderWindow& ventana)
{
    for (int i = 0; i < numJugadores && i < (int)paneles.size(); ++i) {
        ventana.draw(paneles[i].fondo);
        ventana.draw(paneles[i].borde);
        ventana.draw(paneles[i].barraFondo);
        ventana.draw(paneles[i].barraVel);
        ventana.draw(paneles[i].textoVel);
        ventana.draw(paneles[i].iconoGancho);
        ventana.draw(paneles[i].textoGancho);
        for (auto& c : paneles[i].circulosVida) ventana.draw(c);
    }

    if (animVictoria) {
        ventana.draw(fondoOsc);
        ventana.draw(textoVict);
        if (finJuego && texGanadorCargada)
            ventana.draw(sprGanador);
        ventana.draw(textoSubVict);
    }
}

void HUD::mostrarSobrevivio(int idSobreviviente)
{
    animVictoria  = true;
    finJuego      = false;
    timerVictoria = 0.f;
    escalaVict    = 0.f;
    texGanadorCargada = false;

    std::string nombre = (idSobreviviente == 0) ? "JUGADOR 1" : "JUGADOR 2";
    textoVict.setString(nombre + "\nSOBREVIVIO!");
    textoVict.setFillColor(colorJugador(idSobreviviente));
    textoSubVict.setString("Presiona A / Enter para continuar");
}

void HUD::mostrarFinJuego(int idGanador)
{
    animVictoria  = true;
    finJuego      = true;
    timerVictoria = 0.f;
    escalaVict    = 0.f;

    std::string nombre = (idGanador == 0) ? "JUGADOR 1" : "JUGADOR 2";
    textoVict.setString("¡" + nombre + "\nGANA EL JUEGO!");
    textoVict.setFillColor(colorJugador(idGanador));
    textoSubVict.setString("Presiona A / Enter para reiniciar");

    // Cargar imagen del ganador (primera celda del spritesheet)
    std::string rutaImg = (idGanador == 0) ? "assets/images/jugador1.png"
                                           : "assets/images/jugador2.png";
    texGanadorCargada = texGanador.loadFromFile(rutaImg,
        sf::IntRect(0, 0, 200, 220)); // primera celda idle
    if (texGanadorCargada) {
        sprGanador.setTexture(texGanador);
        sprGanador.setTextureRect(sf::IntRect(0, 0, 200, 220));
        sprGanador.setScale(2.5f, 2.5f);
        // Marco del color del jugador
        sprGanador.setColor(sf::Color::White);
    }
}

void HUD::ocultarVictoria() {
    animVictoria  = false;
    timerVictoria = 0.f;
    fondoOsc.setFillColor(sf::Color(0,0,0,0));
    texGanadorCargada = false;
}

sf::Color HUD::colorJugador(int id) const {
    return id == 0 ? sf::Color(80, 180, 255) : sf::Color(255, 90, 70);
}
