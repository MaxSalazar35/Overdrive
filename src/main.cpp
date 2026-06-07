// ============================================================
// main.cpp — Overdrive v12
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
#include "Menu.hpp"
#include "Countdown.hpp"
#include "Pausa.hpp"
#include "Audio.hpp"

static const int   VIDAS_INICIALES = 3;
static const float GRACIA_INICIO   = 1.2f;
static const float CAJA_FRENO      = 0.55f;

enum class Estado { Conteo, Jugando, Pausado, EntreRespawn, FinJuego };

static bool accionConfirmar() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
        || sf::Joystick::isButtonPressed(0, BTN_A)
        || sf::Joystick::isButtonPressed(1, BTN_A);
}

static std::vector<b2BodyId> construirListaGancheables(Mapa&) { return {}; }

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow ventana(
        sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA),
        "Overdrive", sf::Style::Close | sf::Style::Titlebar);
    ventana.setFramerateLimit(FPS_LIMITE);
    ventana.setKeyRepeatEnabled(false);

    // ── Sistema de audio ──────────────────────────────────────
    SistemaAudio audio;
    audio.iniciarMusica();

    // ── MENÚ PRINCIPAL ────────────────────────────────────────
    // El menú recibe la música activa para poder mutearla desde Ajustes
    Menu menu(ventana, audio.getMusicaRef());
    ResultadoMenu resMenu = menu.ejecutar();
    if (resMenu.salir || !ventana.isOpen()) return 0;

    // ── Recursos compartidos ──────────────────────────────────
    SistemaParticulas particulas;
    Camara            camara(3840.f * 4, (float)ALTO_VENTANA);
    HUD               hud(ventana);
    Countdown         countdown(ventana);

    sf::Font fuentePausa;
    if (!fuentePausa.loadFromFile("assets/fonts/Rajdhani-Bold.ttf"))
        fuentePausa.loadFromFile("assets/fonts/Ring.ttf");
    bool musicaMuteada = false;
    Pausa pausa(ventana, audio.getMusicaRef(), fuentePausa);

    DatosPersonajeElegido datosJugador[2] = { resMenu.datosJ1, resMenu.datosJ2 };

    // ── Box2D ─────────────────────────────────────────────────
    b2WorldDef wd  = b2DefaultWorldDef();
    wd.gravity     = {0.f, GRAVEDAD};
    b2WorldId worldId = b2CreateWorld(&wd);

    std::unique_ptr<Mapa>   mapa;
    std::vector<std::unique_ptr<Jugador>> jugadores;
    std::vector<b2BodyId>   listaGancheables;

    std::vector<int>  vidas     = {VIDAS_INICIALES, VIDAS_INICIALES};
    std::vector<bool> vivoAntes = {true, true};
    float timerGracia  = GRACIA_INICIO;
    float timerPausa   = 0.f;
    bool  confirmarPress = false;
    int   ganadorFinal   = -1;

    Estado estadoJuego = Estado::Conteo;
    sf::Clock reloj;
    float acumulador = 0.f;
    float timerActualizaGancheables = 0.f;

    // ── Reinicio limpio ───────────────────────────────────────
    auto reiniciarPartida = [&]() {
        jugadores.clear();
        listaGancheables.clear();
        mapa.reset();
        b2DestroyWorld(worldId);
        worldId = b2CreateWorld(&wd);
        mapa    = std::make_unique<Mapa>(worldId);

        particulas.limpiar();
        camara.resetearAlInicio();
        vidas        = {VIDAS_INICIALES, VIDAS_INICIALES};
        vivoAntes    = {true, true};
        timerGracia  = GRACIA_INICIO;
        ganadorFinal = -1;
        hud.ocultarVictoria();

        // Detener victoria y retomar música si estaba sonando
        audio.detenerVictoria();

        jugadores.push_back(std::make_unique<Jugador>(
            worldId, mapa->spawnJugador(0, 0.f), 0, particulas,
            datosJugador[0].sprite, datosJugador[0].color));
        jugadores.push_back(std::make_unique<Jugador>(
            worldId, mapa->spawnJugador(1, 0.f), 1, particulas,
            datosJugador[1].sprite, datosJugador[1].color));
        for (auto& j : jugadores)
            j->gancho->setGancheables(&listaGancheables);

        countdown.iniciar();
        estadoJuego = Estado::Conteo;
    };

    auto aplicarPersonajesAlHUD = [&]() {
        hud.configurarPersonaje(0,
            datosJugador[0].color,
            datosJugador[0].alias.empty()   ? "VANDAL" : datosJugador[0].alias,
            datosJugador[0].victory.empty() ? "assets/images/vandal_victory.png" : datosJugador[0].victory);
        hud.configurarPersonaje(1,
            datosJugador[1].color,
            datosJugador[1].alias.empty()   ? "COBALT" : datosJugador[1].alias,
            datosJugador[1].victory.empty() ? "assets/images/cobalt_victory.png" : datosJugador[1].victory);
    };

    aplicarPersonajesAlHUD();
    reiniciarPartida();

    // ── Bucle principal ───────────────────────────────────────
    while (ventana.isOpen())
    {
        // Alternar música si terminó la pista actual
        audio.updateMusica();

        sf::Event ev;
        while (ventana.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) ventana.close();

            if (ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Escape
                && estadoJuego == Estado::Jugando)
            {
                pausa.abrir();
                estadoJuego = Estado::Pausado;
            }

            if (estadoJuego == Estado::Pausado) {
                ResultadoPausa rp = pausa.procesarEvento(ev, musicaMuteada);
                if (rp == ResultadoPausa::Continuar) {
                    estadoJuego = Estado::Jugando;
                } else if (rp == ResultadoPausa::VolverMenu) {
                    jugadores.clear();
                    listaGancheables.clear();
                    mapa.reset();
                    b2DestroyWorld(worldId);
                    worldId = b2CreateWorld(&wd);
                    audio.detenerVictoria();

                    Menu menuPausa(ventana, audio.getMusicaRef());
                    ResultadoMenu rMenu = menuPausa.ejecutar();
                    if (rMenu.salir || !ventana.isOpen()) { ventana.close(); break; }

                    datosJugador[0] = rMenu.datosJ1;
                    datosJugador[1] = rMenu.datosJ2;
                    aplicarPersonajesAlHUD();
                    reiniciarPartida();
                }
            }
        }

        if (!ventana.isOpen()) break;

        float dt = reloj.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        // ── CONTEO ───────────────────────────────────────────
        if (estadoJuego == Estado::Conteo)
        {
            acumulador += dt;
            while (acumulador >= TIEMPO_PASO) {
                b2World_Step(worldId, TIEMPO_PASO, SUB_PASOS);
                acumulador -= TIEMPO_PASO;
            }
            for (auto& j : jugadores) j->update(dt);

            if (!countdown.update(dt)) {
                timerGracia = GRACIA_INICIO * 0.3f;
                estadoJuego = Estado::Jugando;
            }
            hud.update({0.f,0.f},{false,false},vidas,0,dt);
        }
        // ── JUGANDO ──────────────────────────────────────────
        else if (estadoJuego == Estado::Jugando)
        {
            if (timerGracia > 0.f) timerGracia -= dt;

            for (auto& j : jugadores) j->procesarEntrada(dt);

            acumulador += dt;
            while (acumulador >= TIEMPO_PASO) {
                b2World_Step(worldId, TIEMPO_PASO, SUB_PASOS);
                acumulador -= TIEMPO_PASO;
            }
            for (auto& j : jugadores) j->update(dt);

            timerActualizaGancheables += dt;
            if (timerActualizaGancheables > 2.f) {
                timerActualizaGancheables = 0.f;
                listaGancheables = construirListaGancheables(*mapa);
                for (auto& j : jugadores)
                    j->gancho->setGancheables(&listaGancheables);
            }

            mapa->update(dt, camara.getCentro().x);

            // Cajas
            for (auto& j : jugadores) {
                if (j->estaMuerto()) continue;
                sf::FloatRect bj = j->getBounds();
                for (auto& c : mapa->getCajas()) {
                    if (!c.activa) continue;
                    if (bj.intersects(c.visual.getGlobalBounds())) {
                        j->aplicarFreno(CAJA_FRENO);
                        c.activa = false; c.timerRespawn = 0.f;
                        particulas.emitir(c.posBase, TipoParticula::Chispa, sf::Color(255,220,80), 15);
                        audio.playCaja();   // ← sonido de caja
                    }
                }
            }

            // Picos
            if (timerGracia <= 0.f) {
                for (auto& j : jugadores) {
                    if (j->estaMuerto()) continue;
                    sf::FloatRect bj = j->getBounds();
                    for (auto& p : mapa->getPlataformas()) {
                        if (!p.esPico || !p.valido) continue;
                        if (bj.intersects(p.rectVisual.getGlobalBounds())) {
                            j->eliminar();
                            particulas.emitir(j->getPosicion(), TipoParticula::Chispa, sf::Color(255,60,60), 25);
                            audio.playDeath();  // ← sonido de muerte por pico
                        }
                    }
                }
            }

            // Cámara
            std::vector<sf::Vector2f> posVivos;
            for (auto& j : jugadores)
                if (!j->estaMuerto()) posVivos.push_back(j->getPosicion());
            if (!posVivos.empty()) camara.update(posVivos, dt);

            // Eliminación por salir de pantalla
            if (timerGracia <= 0.f) {
                for (auto& j : jugadores) {
                    if (!j->estaMuerto() && camara.fueraDePantalla(j->getPosicion())) {
                        j->eliminar();
                        particulas.emitir(j->getPosicion(), TipoParticula::Confetti, sf::Color::White, 30);
                        audio.playDeath();  // ← sonido de muerte por caída
                    }
                }
            }

            particulas.update(dt);

            // Detectar muerte
            int idMuerto = -1;
            for (int i = 0; i < (int)jugadores.size(); ++i) {
                bool muertoAhora = jugadores[i]->estaMuerto();
                if (vivoAntes[i] && muertoAhora) idMuerto = i;
                vivoAntes[i] = !muertoAhora;
            }

            if (idMuerto >= 0) {
                vidas[idMuerto]--;
                int idSob = 1 - idMuerto;
                particulas.emitir({(float)ANCHO_VENTANA/2.f,(float)ALTO_VENTANA/2.f},
                                   TipoParticula::Confetti, sf::Color::White, 60);
                if (vidas[idMuerto] <= 0) {
                    ganadorFinal = idSob;
                    hud.mostrarFinJuego(ganadorFinal);
                    audio.iniciarVictoria();  // ← musica de victoria
                    estadoJuego = Estado::FinJuego;
                } else {
                    hud.mostrarSobrevivio(idSob);
                    estadoJuego = Estado::EntreRespawn;
                    timerPausa  = 0.f;
                }
            }

            // HUD
            std::vector<float> vels; std::vector<bool> ganch;
            int idLider = 0; float maxX = -1e9f;
            for (int i = 0; i < (int)jugadores.size(); ++i) {
                vels.push_back(jugadores[i]->getVelocidadX());
                ganch.push_back(jugadores[i]->gancho->estaActivo());
                if (jugadores[i]->getPosicion().x > maxX)
                    { maxX = jugadores[i]->getPosicion().x; idLider = i; }
            }
            hud.update(vels, ganch, vidas, idLider, dt);
        }
        // ── PAUSADO ──────────────────────────────────────────
        else if (estadoJuego == Estado::Pausado)
        {
            pausa.update(dt);
            hud.update({0.f,0.f},{false,false},vidas,0,dt);
            particulas.update(dt);
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
                float camX = camara.getCentro().x;
                for (int i = 0; i < (int)jugadores.size(); ++i)
                    jugadores[i]->resetear(mapa->spawnJugador(i, camX));
                vivoAntes   = {true, true};
                timerGracia = GRACIA_INICIO;
                camara.activarShake(0.5f, 14.f);
                countdown.iniciar();
                estadoJuego = Estado::Conteo;
            }
            confirmarPress = conf;
            hud.update({0.f,0.f},{false,false},vidas,0,dt);
            particulas.update(dt);
        }
        // ── FIN DE JUEGO ──────────────────────────────────────
        else if (estadoJuego == Estado::FinJuego)
        {
            bool conf   = accionConfirmar();
            bool escNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

            if (conf && !confirmarPress) {
                reiniciarPartida();   // detiene victoria internamente
                confirmarPress = false;
            } else if (escNow && !confirmarPress) {
                jugadores.clear();
                listaGancheables.clear();
                mapa.reset();
                b2DestroyWorld(worldId);
                worldId = b2CreateWorld(&wd);
                audio.detenerVictoria();

                Menu menuRematch(ventana, audio.getMusicaRef());
                ResultadoMenu res2 = menuRematch.ejecutar();
                if (res2.salir || !ventana.isOpen()) break;

                datosJugador[0] = res2.datosJ1;
                datosJugador[1] = res2.datosJ2;
                aplicarPersonajesAlHUD();
                reiniciarPartida();
                confirmarPress = false;
            } else {
                confirmarPress = conf || escNow;
            }

            hud.update({0.f,0.f},{false,false},vidas,0,dt);
            particulas.update(dt);
        }

        // ── RENDER ───────────────────────────────────────────
        if (estadoJuego != Estado::FinJuego)
            camara.aplicar(ventana);

        ventana.clear(sf::Color(10,12,30));

        if (mapa) {
            mapa->drawFondo(ventana, camara.getCentro().x);
            mapa->drawPlataformas(ventana);
            mapa->drawItems(ventana);
        }

        if (estadoJuego != Estado::FinJuego)
            for (auto& j : jugadores) j->draw(ventana);

        particulas.draw(ventana);

        camara.restaurarVista(ventana);
        hud.draw(ventana);

        if (estadoJuego == Estado::Conteo)
            countdown.draw();

        if (estadoJuego == Estado::Pausado)
            pausa.draw(musicaMuteada);

        ventana.display();
    }

    jugadores.clear();
    listaGancheables.clear();
    mapa.reset();
    b2DestroyWorld(worldId);
    return 0;
}
