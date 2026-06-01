// ============================================================
// main.cpp — Overdrive (Box2D v3)
// Sistema de vidas: 3 vidas por jugador.
// Al morir uno → pantalla "sobrevivió", luego ambos respawnean
// en el inicio con la cámara reseteada.
// Al quedarse sin vidas → pantalla de fin de juego con imagen.
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <box2d/box2d.h>
#include <memory>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "Constantes.hpp"
#include "Jugador.hpp"
#include "Mapa.hpp"
#include "Camara.hpp"
#include "HUD.hpp"
#include "Particulas.hpp"

static const int   VIDAS_INICIALES  = 3;
static const float GRACIA_INICIO    = 1.2f;  // segundos sin poder morir al respawnear

enum class Estado { Jugando, EntreRespawn, FinJuego };

static bool accionConfirmar() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
        || sf::Joystick::isButtonPressed(0, BTN_A)
        || sf::Joystick::isButtonPressed(1, BTN_A);
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow ventana(
        sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA),
        "Overdrive",
        sf::Style::Close | sf::Style::Titlebar
    );
    ventana.setFramerateLimit(FPS_LIMITE);
    ventana.setKeyRepeatEnabled(false);

    // ── Música ───────────────────────────────────────────────
    sf::Music musica;
    if (musica.openFromFile("assets/music/musica.ogg")) {
        musica.setLoop(true);
        musica.setVolume(45.f);
        musica.play();
    }

    // ── Box2D v3 ─────────────────────────────────────────────
    b2WorldDef wd = b2DefaultWorldDef();
    wd.gravity    = {0.f, GRAVEDAD};
    b2WorldId worldId = b2CreateWorld(&wd);

    // ── Sistemas ─────────────────────────────────────────────
    SistemaParticulas particulas;
    Mapa   mapa(worldId);
    Camara camara(mapa.getAncho(), mapa.getAlto());
    HUD    hud(ventana);

    // ── Estado de vidas ──────────────────────────────────────
    std::vector<int>  vidas      = {VIDAS_INICIALES, VIDAS_INICIALES};
    // vivoAntes: detectar transición vivo→muerto (solo contar 1 vez)
    std::vector<bool> vivoAntes  = {true, true};
    // Timer de gracia: primeros segundos tras respawn no se puede morir
    float timerGracia = GRACIA_INICIO;

    // ── Crear jugadores (inicio o reinicio completo) ──────────
    auto crearJugadores = [&]() {
        std::vector<std::unique_ptr<Jugador>> jgs;
        jgs.push_back(std::make_unique<Jugador>(
            worldId, mapa.spawnJugador(0), 0, particulas));
        jgs.push_back(std::make_unique<Jugador>(
            worldId, mapa.spawnJugador(1), 1, particulas));
        vivoAntes   = {true, true};
        timerGracia = GRACIA_INICIO;
        camara.resetearAlInicio();
        return jgs;
    };

    // ── Respawnear (entre vidas: mismos objetos, posición reset) ─
    auto respawnearJugadores = [&](std::vector<std::unique_ptr<Jugador>>& jgs) {
        for (int i = 0; i < (int)jgs.size(); ++i)
            jgs[i]->resetear(mapa.spawnJugador(i));
        vivoAntes   = {true, true};
        timerGracia = GRACIA_INICIO;  // gracia tras respawn
        camara.resetearAlInicio();    // cámara vuelve al inicio
    };

    auto jugadores = crearJugadores();

    Estado estadoJuego   = Estado::Jugando;
    float  timerPausa    = 0.f;
    bool   confirmarPress= false;
    int    ganadorFinal  = -1;

    sf::Clock reloj;
    float acumulador = 0.f;

    while (ventana.isOpen())
    {
        sf::Event ev;
        while (ventana.pollEvent(ev))
            if (ev.type == sf::Event::Closed) ventana.close();

        float dt = reloj.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        // ── JUGANDO ──────────────────────────────────────────
        if (estadoJuego == Estado::Jugando)
        {
            // Bajar timer de gracia
            if (timerGracia > 0.f) timerGracia -= dt;

            for (auto& j : jugadores) j->procesarEntrada(dt);

            acumulador += dt;
            while (acumulador >= TIEMPO_PASO) {
                b2World_Step(worldId, TIEMPO_PASO, SUB_PASOS);
                acumulador -= TIEMPO_PASO;
            }

            for (auto& j : jugadores) j->update(dt);
            mapa.update(dt);

            // Colisión con cajas
            for (auto& j : jugadores) {
                sf::FloatRect bj = j->getBounds();
                for (auto& c : mapa.getCajas()) {
                    if (!c.activa) continue;
                    if (bj.intersects(c.visual.getGlobalBounds())) {
                        c.activa = false; c.timerRespawn = 0.f;
                        particulas.emitir(c.posBase, TipoParticula::Chispa,
                                          sf::Color(255,220,80), 15);
                    }
                }
            }

            // Cámara — solo seguir a jugadores vivos
            std::vector<sf::Vector2f> posVivos;
            for (auto& j : jugadores)
                if (!j->estaMuerto())
                    posVivos.push_back(j->getPosicion());
            if (!posVivos.empty())
                camara.update(posVivos, dt);

            // Eliminaciones por salirse de pantalla — solo si pasó la gracia
            if (timerGracia <= 0.f) {
                for (auto& j : jugadores) {
                    if (!j->estaMuerto() && camara.fueraDePantalla(j->getPosicion())) {
                        j->eliminar();
                        particulas.emitir(j->getPosicion(),
                                          TipoParticula::Confetti, sf::Color::White, 30);
                    }
                }
            }

            particulas.update(dt);

            // ── Detectar muerte: solo transición vivo→muerto ─
            int idMuerto = -1;
            for (int i = 0; i < (int)jugadores.size(); ++i) {
                bool muertoAhora = jugadores[i]->estaMuerto();
                if (vivoAntes[i] && muertoAhora)
                    idMuerto = i;
                vivoAntes[i] = !muertoAhora;
            }

            if (idMuerto >= 0) {
                vidas[idMuerto]--;
                int idSobreviviente = 1 - idMuerto;

                particulas.emitir(
                    {(float)ANCHO_VENTANA/2.f, (float)ALTO_VENTANA/2.f},
                    TipoParticula::Confetti, sf::Color::White, 60);

                if (vidas[idMuerto] <= 0) {
                    ganadorFinal = idSobreviviente;
                    hud.mostrarFinJuego(ganadorFinal);
                    estadoJuego = Estado::FinJuego;
                } else {
                    hud.mostrarSobrevivio(idSobreviviente);
                    estadoJuego = Estado::EntreRespawn;
                    timerPausa  = 0.f;
                }
            }

            // HUD
            std::vector<float> vels;
            std::vector<bool>  ganch;
            int idLider = 0;
            float maxX  = -1e9f;
            for (int i = 0; i < (int)jugadores.size(); ++i) {
                vels.push_back(jugadores[i]->getVelocidadX());
                ganch.push_back(jugadores[i]->gancho->estaActivo());
                if (jugadores[i]->getPosicion().x > maxX) {
                    maxX = jugadores[i]->getPosicion().x;
                    idLider = i;
                }
            }
            hud.update(vels, ganch, vidas, idLider, dt);
        }
        // ── ENTRE RESPAWN ─────────────────────────────────────
        else if (estadoJuego == Estado::EntreRespawn)
        {
            timerPausa += dt;
            bool conf = accionConfirmar();
            if ((conf && !confirmarPress && timerPausa > 1.f)
                || timerPausa > PAUSA_ENTRE_RONDAS + 2.f)
            {
                hud.ocultarVictoria();
                particulas.limpiar();
                respawnearJugadores(jugadores);
                estadoJuego = Estado::Jugando;
            }
            confirmarPress = conf;
            hud.update({0.f,0.f},{false,false},vidas,0,dt);
            particulas.update(dt);
        }
        // ── FIN DE JUEGO ──────────────────────────────────────
        else if (estadoJuego == Estado::FinJuego)
        {
            bool conf = accionConfirmar();
            if (conf && !confirmarPress) {
                vidas        = {VIDAS_INICIALES, VIDAS_INICIALES};
                ganadorFinal = -1;
                hud.ocultarVictoria();
                particulas.limpiar();
                jugadores.clear();
                jugadores = crearJugadores();
                estadoJuego = Estado::Jugando;
            }
            confirmarPress = conf;
            hud.update({0.f,0.f},{false,false},vidas,0,dt);
            particulas.update(dt);
        }

        // ── Render ───────────────────────────────────────────
        camara.aplicar(ventana);
        ventana.clear(sf::Color(10,12,30));

        mapa.drawFondo(ventana, camara.getCentro().x);
        mapa.drawPlataformas(ventana);
        mapa.drawItems(ventana);
        for (auto& j : jugadores) j->draw(ventana);
        particulas.draw(ventana);

        camara.restaurarVista(ventana);
        hud.draw(ventana);

        ventana.display();
    }

    b2DestroyWorld(worldId);
    return 0;
}
