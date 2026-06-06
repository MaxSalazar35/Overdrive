// ============================================================
// HUD.cpp — Overdrive
// Fuente: Rajdhani-Bold.ttf
// Panel por jugador:
//   - Nombre "JUGADOR 1 / 2" en grande
//   - Velocidad numérica en km/h
//   - Barra de velocidad con color dinámico
//   - Vidas como círculos
//   - Indicador de gancho activo
// ============================================================
#include "HUD.hpp"
#include <cmath>
#include <string>

static const int VIDAS_MAX = 3;

HUD::HUD(sf::RenderWindow& ventana)
{
    anchoV = (float)ventana.getSize().x;
    altoV  = (float)ventana.getSize().y;

    // Intentar cargar Rajdhani, si no hay usar Ring
    if (!fuente.loadFromFile("assets/fonts/Rajdhani-Bold.ttf")) {
        fuenteCargada = fuente.loadFromFile("assets/fonts/Ring.ttf");
    } else {
        fuenteCargada = true;
    }

    std::vector<int> vidasInic(2, VIDAS_MAX);
    inicializarPaneles(vidasInic);

    fondoOsc.setSize({anchoV, altoV});
    fondoOsc.setFillColor(sf::Color(0, 0, 0, 0));

    textoVict.setFont(fuente);
    textoVict.setCharacterSize(80);
    textoVict.setOutlineThickness(4.f);
    textoVict.setOutlineColor(sf::Color::Black);

    textoSubVict.setFont(fuente);
    textoSubVict.setCharacterSize(32);
    textoSubVict.setFillColor(sf::Color(200, 200, 200));
    textoSubVict.setString("Presiona Space o Enter para continuar");

    sprGanador.setScale(2.5f, 2.5f);
}

void HUD::inicializarPaneles(const std::vector<int>& vidasIniciales)
{
    paneles.clear();
    for (int i = 0; i < numJugadores; ++i) {
        Panel p;
        sf::Color cj = colorJugador(i);

        // Panel más ancho y alto para el nuevo diseño
        float panelW = 230.f;
        float panelH = 110.f;
        float xBase  = (i == 0) ? 10.f : anchoV - panelW - 10.f;
        float yBase  = 10.f;

        // Fondo semitransparente con borde del color del jugador
        p.fondo.setSize({panelW, panelH});
        p.fondo.setPosition(xBase, yBase);
        p.fondo.setFillColor(sf::Color(8, 10, 22, 190));

        p.borde.setSize({panelW, panelH});
        p.borde.setPosition(xBase, yBase);
        p.borde.setFillColor(sf::Color::Transparent);
        p.borde.setOutlineColor(cj);
        p.borde.setOutlineThickness(2.f);

        // ── Nombre del jugador ───────────────────────────────
        p.textoNombre.setFont(fuente);
        p.textoNombre.setCharacterSize(22);
        p.textoNombre.setFillColor(cj);
        p.textoNombre.setString(i == 0 ? "JUGADOR 1" : "JUGADOR 2");
        p.textoNombre.setPosition(xBase + 10.f, yBase + 6.f);

        // ── Velocidad numérica ───────────────────────────────
        p.textoVel.setFont(fuente);
        p.textoVel.setCharacterSize(28);
        p.textoVel.setFillColor(sf::Color::White);
        p.textoVel.setPosition(xBase + 10.f, yBase + 30.f);
        p.textoVel.setString("0 km/h");

        // ── Barra de velocidad ───────────────────────────────
        p.barraFondo.setSize({panelW - 20.f, 8.f});
        p.barraFondo.setPosition(xBase + 10.f, yBase + 64.f);
        p.barraFondo.setFillColor(sf::Color(30, 30, 50));

        p.barraVel.setSize({0.f, 8.f});
        p.barraVel.setPosition(xBase + 10.f, yBase + 64.f);
        p.barraVel.setFillColor(cj);

        // ── Indicador gancho ─────────────────────────────────
        p.iconoGancho.setRadius(6.f);
        p.iconoGancho.setOrigin(6.f, 6.f);
        p.iconoGancho.setPosition(xBase + panelW - 16.f, yBase + 16.f);
        p.iconoGancho.setFillColor(sf::Color(40, 40, 60));
        p.iconoGancho.setOutlineColor(sf::Color(255, 220, 80));
        p.iconoGancho.setOutlineThickness(1.5f);

        p.textoGancho.setFont(fuente);
        p.textoGancho.setCharacterSize(10);
        p.textoGancho.setFillColor(sf::Color(180, 180, 180));
        p.textoGancho.setString("HOOK");
        float gw = p.textoGancho.getLocalBounds().width;
        p.textoGancho.setPosition(xBase + panelW - 16.f - gw/2.f, yBase + 26.f);

        // ── Vidas (círculos) ─────────────────────────────────
        int vMax = (i < (int)vidasIniciales.size()) ? vidasIniciales[i] : VIDAS_MAX;
        for (int r = 0; r < vMax; ++r) {
            sf::CircleShape c(7.f);
            c.setOrigin(7.f, 7.f);
            c.setPosition(xBase + 14.f + r * 22.f, yBase + 88.f);
            c.setFillColor(cj);
            c.setOutlineColor(sf::Color(255, 255, 255, 100));
            c.setOutlineThickness(1.f);
            p.circulosVida.push_back(c);
        }

        // ── Corona de líder ──────────────────────────────────
        p.corona.setFont(fuente);
        p.corona.setCharacterSize(14);
        p.corona.setFillColor(sf::Color(255, 215, 0));
        p.corona.setString(">> LIDER <<");
        p.corona.setPosition(xBase + panelW - 100.f, yBase + 88.f);

        paneles.push_back(p);
    }
}

void HUD::update(
    const std::vector<float>& velocidades,
    const std::vector<bool>&  ganchosActivos,
    const std::vector<int>&   vidas,
    int idLider, float dt)
{
    const float VEL_MAX_VIS = 400.f;

    for (int i = 0; i < numJugadores && i < (int)paneles.size(); ++i) {
        float velAbs = std::abs(velocidades[i]);
        float velKmh = velAbs * 0.36f;
        float pct    = std::min(velAbs / VEL_MAX_VIS, 1.f);

        // Velocidad numérica
        paneles[i].textoVel.setString(std::to_string((int)velKmh) + " km/h");

        // Barra — va de azul/rojo del jugador a naranja/rojo cuando está al límite
        float panelW = paneles[i].barraFondo.getSize().x;
        paneles[i].barraVel.setSize({pct * panelW, 8.f});

        sf::Color cj = colorJugador(i);
        if (pct > 0.85f) {
            // Rojo brillante cuando está al límite
            paneles[i].barraVel.setFillColor(sf::Color(255, 60, 60));
        } else if (pct > 0.6f) {
            paneles[i].barraVel.setFillColor(sf::Color(255, 160, 40));
        } else {
            paneles[i].barraVel.setFillColor(cj);
        }

        // Gancho
        paneles[i].iconoGancho.setFillColor(
            ganchosActivos[i] ? sf::Color(255, 220, 80) : sf::Color(30, 30, 50)
        );

        // Vidas
        int v = (i < (int)vidas.size()) ? vidas[i] : 0;
        for (int r = 0; r < (int)paneles[i].circulosVida.size(); ++r) {
            bool activa = r < v;
            paneles[i].circulosVida[r].setFillColor(
                activa ? colorJugador(i) : sf::Color(25, 25, 40)
            );
        }

        // Corona — solo al líder
        paneles[i].corona.setFillColor(
            (i == idLider) ? sf::Color(255, 215, 0) : sf::Color::Transparent
        );
    }

    // Animación de victoria
    if (animVictoria) {
        timerVictoria += dt;
        sf::Uint8 alfa = (sf::Uint8)std::min(timerVictoria * 400.f, 220.f);  // más opaco y rápido
        fondoOsc.setFillColor(sf::Color(0, 0, 0, alfa));
        escalaVict = std::min(1.f, timerVictoria * 4.f);
        float s = escalaVict < 0.8f
            ? escalaVict / 0.8f * 1.2f
            : 1.2f - (escalaVict - 0.8f) / 0.2f * 0.2f;
        textoVict.setScale(s, s);

        sf::FloatRect b = textoVict.getLocalBounds();
        textoVict.setOrigin(b.width/2.f, b.height/2.f);

        if (finJuego) {
            // Imagen del ganador a la izquierda, texto a la derecha
            if (texGanadorCargada) {
                sf::FloatRect sb = sprGanador.getLocalBounds();
                sprGanador.setOrigin(sb.width/2.f, sb.height/2.f);
                sprGanador.setPosition(anchoV * 0.28f, altoV * 0.50f);
                // Escala fija — la imagen ya tiene el tamaño correcto desde mostrarFinJuego
                (void)s; // sin bounce en la imagen, solo el texto lo tiene
            }
            // Texto a la derecha, centrado verticalmente
            textoVict.setPosition(anchoV * 0.65f, altoV * 0.38f);
        } else {
            textoVict.setPosition(anchoV/2.f, altoV * 0.42f);
        }

        sf::FloatRect b2 = textoSubVict.getLocalBounds();
        textoSubVict.setOrigin(b2.width/2.f, b2.height/2.f);
        // En fin de juego, subtexto debajo del texto de victoria (derecha)
        // En entre-rondas, centrado
        float subX = finJuego ? anchoV * 0.65f : anchoV / 2.f;
        float subY = finJuego ? altoV * 0.68f  : altoV * 0.80f;
        textoSubVict.setPosition(subX, subY);
        sf::Uint8 alfaSub = (sf::Uint8)(std::sin(timerVictoria * 4.f) * 80.f + 160.f);
        textoSubVict.setFillColor(sf::Color(200,200,200,alfaSub));
    }
}

void HUD::draw(sf::RenderWindow& ventana)
{
    for (int i = 0; i < numJugadores && i < (int)paneles.size(); ++i) {
        ventana.draw(paneles[i].fondo);
        ventana.draw(paneles[i].borde);
        ventana.draw(paneles[i].textoNombre);
        ventana.draw(paneles[i].textoVel);
        ventana.draw(paneles[i].barraFondo);
        ventana.draw(paneles[i].barraVel);
        ventana.draw(paneles[i].iconoGancho);
        ventana.draw(paneles[i].textoGancho);
        for (auto& c : paneles[i].circulosVida) ventana.draw(c);
        ventana.draw(paneles[i].corona);
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
    // Sin acentos para evitar problemas de encoding
    textoVict.setString(nombre + "\nGANA LA RONDA!");
    textoVict.setFillColor(colorJugador(idSobreviviente));
    textoSubVict.setString("Presiona Space o Enter para continuar");
}

void HUD::mostrarFinJuego(int idGanador)
{
    animVictoria  = true;
    finJuego      = true;
    timerVictoria = 0.f;
    escalaVict    = 0.f;

    std::string nombre = (idGanador == 0) ? "JUGADOR 1" : "JUGADOR 2";
    textoVict.setString("!" + nombre + "\nGANA EL JUEGO!");
    textoVict.setFillColor(colorJugador(idGanador));
    textoSubVict.setString("Presiona Space o Enter para reiniciar");

    // Usar imágenes de victoria con fondo removido (Photoroom)
    std::string rutaImg = (idGanador == 0)
        ? "assets/images/jugador1victory-Photoroom.png"
        : "assets/images/jugador2victory-Photoroom.png";
    texGanadorCargada = texGanador.loadFromFile(rutaImg);
    if (texGanadorCargada) {
        sprGanador.setTexture(texGanador, true);
        // Imagen 1536x1024 → escalar para que quepa en ~40% de la pantalla de alto
        sf::Vector2u sz = texGanador.getSize();
        float escala = (altoV * 0.65f) / (float)sz.y;
        sprGanador.setScale(escala, escala);
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
