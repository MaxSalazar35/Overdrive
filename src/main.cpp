// ============================================================
// main.cpp — Overdrive (Box2D v3)
// Novedades v4 (menú):
//   - Menú principal con Play / Cómo Jugar / Ajustes / Salir
//   - Selección de personaje por jugador con lore
//   - Cajas del jugador frena al tocarlas (no bloquean)
//   - Picos en el mapa matan al jugador
//   - Gancho solo en superficies marcadas
//   - Desliz en bajada agrega impulso
//   - HUD con nombre, velocidad en Rajdhani-Bold
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

static const int   VIDAS_INICIALES  = 3;
static const float GRACIA_INICIO    = 1.2f;

// Cuánto se frena al tocar una caja del jugador (factor por frame)
static const float CAJA_FRENO       = 0.55f;

enum class Estado { Jugando, EntreRespawn, FinJuego };

static bool accionConfirmar() {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)
        || sf::Joystick::isButtonPressed(0, BTN_A)
        || sf::Joystick::isButtonPressed(1, BTN_A);
}

// ── Lista de gancheables (ya no se usa para filtrar — el gancho filtra por posición Y)
// Se mantiene para compatibilidad. Pasar nullptr a setGancheables desactiva el filtro por lista.
static std::vector<b2BodyId> construirListaGancheables(Mapa& /*mapa*/)
{
    return {};  // Lista vacía: el gancho usa filtro por posición (y < 580) en su callback
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
    bool musicaOK = musica.openFromFile("assets/music/musica.ogg");
    if (musicaOK) {
        musica.setLoop(true);
        musica.setVolume(50.f);
        musica.play();
    }

    // ── MENÚ PRINCIPAL ────────────────────────────────────────
    Menu menu(ventana, musica);
    ResultadoMenu resMenu = menu.ejecutar();

    if (resMenu.salir || !ventana.isOpen())
        return 0;

    // ── Box2D v3 ─────────────────────────────────────────────
    b2WorldDef wd = b2DefaultWorldDef();
    wd.gravity    = {0.f, GRAVEDAD};
    b2WorldId worldId = b2CreateWorld(&wd);

    // ── Sistemas ─────────────────────────────────────────────
    SistemaParticulas particulas;
    Mapa   mapa(worldId);
    Camara camara(mapa.getAncho(), mapa.getAlto());
    HUD    hud(ventana);

    // ── Lista de bodies gancheables (se reconstruye si cambia el mapa) ──
    std::vector<b2BodyId> listaGancheables = construirListaGancheables(mapa);

    // ── Estado de vidas ──────────────────────────────────────
    std::vector<int>  vidas     = {VIDAS_INICIALES, VIDAS_INICIALES};
    std::vector<bool> vivoAntes = {true, true};
    float timerGracia = GRACIA_INICIO;

    auto crearJugadores = [&]() {
        std::vector<std::unique_ptr<Jugador>> jgs;
        jgs.push_back(std::make_unique<Jugador>(
            worldId, mapa.spawnJugador(0, 0.f), 0, particulas));
        jgs.push_back(std::make_unique<Jugador>(
            worldId, mapa.spawnJugador(1, 0.f), 1, particulas));
        // Registrar lista de gancheables en cada gancho
        for (auto& j : jgs)
            j->gancho->setGancheables(&listaGancheables);
        vivoAntes   = {true, true};
        timerGracia = GRACIA_INICIO;
        camara.resetearAlInicio();
        return jgs;
    };

    auto respawnearJugadores = [&](std::vector<std::unique_ptr<Jugador>>& jgs) {
        float camX = camara.getCentro().x;
        for (int i = 0; i < (int)jgs.size(); ++i)
            jgs[i]->resetear(mapa.spawnJugador(i, camX));
        vivoAntes   = {true, true};
        timerGracia = GRACIA_INICIO;
    };

    auto jugadores = crearJugadores();

    Estado estadoJuego    = Estado::Jugando;
    float  timerPausa     = 0.f;
    bool   confirmarPress = false;
    int    ganadorFinal   = -1;

    sf::Clock reloj;
    float acumulador = 0.f;

    // ── Timer para reconstruir lista de gancheables ──────────
    float timerActualizaGancheables = 0.f;

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
            if (timerGracia > 0.f) timerGracia -= dt;

            for (auto& j : jugadores) j->procesarEntrada(dt);

            acumulador += dt;
            while (acumulador >= TIEMPO_PASO) {
                b2World_Step(worldId, TIEMPO_PASO, SUB_PASOS);
                acumulador -= TIEMPO_PASO;
            }

            for (auto& j : jugadores) j->update(dt);

            // ── Reconstruir lista de gancheables periódicamente ──
            // (cuando se generan/eliminan chunks cambian los bodies)
            timerActualizaGancheables += dt;
            if (timerActualizaGancheables > 2.f) {
                timerActualizaGancheables = 0.f;
                listaGancheables = construirListaGancheables(mapa);
                for (auto& j : jugadores)
                    j->gancho->setGancheables(&listaGancheables);
            }

            mapa.update(dt, camara.getCentro().x);

            // ── Colisión con cajas del pickup ─────────────────
            // Las cajas FRENA al jugador que las toca (no bloquean)
            for (auto& j : jugadores) {
                if (j->estaMuerto()) continue;
                sf::FloatRect bj = j->getBounds();
                for (auto& c : mapa.getCajas()) {
                    if (!c.activa) continue;
                    if (bj.intersects(c.visual.getGlobalBounds())) {
                        // Frenar velocidad X del jugador
                        j->aplicarFreno(CAJA_FRENO);
                        c.activa       = false;
                        c.timerRespawn = 0.f;
                        particulas.emitir(c.posBase, TipoParticula::Chispa,
                                          sf::Color(255,220,80), 15);
                    }
                }
            }

            // ── Colisión con picos ────────────────────────────
            if (timerGracia <= 0.f) {
                for (auto& j : jugadores) {
                    if (j->estaMuerto()) continue;
                    sf::FloatRect bj = j->getBounds();
                    for (auto& p : mapa.getPlataformas()) {
                        if (!p.esPico || !p.valido) continue;
                        sf::FloatRect bp = p.rectVisual.getGlobalBounds();
                        if (bj.intersects(bp)) {
                            // Muere instantáneamente
                            j->eliminar();
                            particulas.emitir(j->getPosicion(),
                                TipoParticula::Chispa, sf::Color(255, 60, 60), 25);
                        }
                    }
                }
            }

            // Cámara
            std::vector<sf::Vector2f> posVivos;
            for (auto& j : jugadores)
                if (!j->estaMuerto())
                    posVivos.push_back(j->getPosicion());
            if (!posVivos.empty())
                camara.update(posVivos, dt);

            // Eliminación por salir de pantalla
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

            // ── Detectar muerte ───────────────────────────────
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
                // Destruir mundo físico antes del menú
                b2DestroyWorld(worldId);

                // Volver al menú principal
                Menu menuRematch(ventana, musica);
                ResultadoMenu resRematch = menuRematch.ejecutar();

                if (resRematch.salir || !ventana.isOpen())
                    break;

                // Recrear mundo físico con configuración nueva
                wd.gravity = {0.f, GRAVEDAD};
                worldId = b2CreateWorld(&wd);

                vidas        = {VIDAS_INICIALES, VIDAS_INICIALES};
                ganadorFinal = -1;
                hud.ocultarVictoria();
                particulas.limpiar();
                mapa = Mapa(worldId);
                camara.resetearAlInicio();
                jugadores.clear();
                listaGancheables.clear();
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
