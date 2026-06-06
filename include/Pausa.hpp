#pragma once
// ============================================================
// Pausa.hpp — Overdrive
// Menú de pausa en partida. Se activa con Escape.
// Opciones: Continuar / Cómo Jugar / Ajustes / Volver al Menú
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <cmath>

enum class ResultadoPausa {
    Ninguno,       // sigue pausado
    Continuar,     // cerrar pausa, seguir jugando
    VolverMenu     // salir al menú principal
};

class Pausa {
public:
    Pausa(sf::RenderWindow& v, sf::Music& m, sf::Font& f)
        : ventana(v), musica(m), fuente(f)
    {}

    bool estaActivo() const { return activo; }

    void abrir()  {
        activo = true;
        opcion = 0;
        tiempoApertura = 0.f;
        // Guardar estado de música
        musica.setVolume(20.f);   // bajar música al pausar
    }
    void cerrar() {
        activo = false;
        musica.setVolume(50.f);   // restaurar volumen
    }

    // Llama en el bucle principal cuando está pausado.
    // Devuelve la acción elegida.
    ResultadoPausa update(float dt, bool& musicaMuteada, bool& pantallaCompleta) {
        if (!activo) return ResultadoPausa::Ninguno;
        tiempoApertura += dt;

        // Navegación
        bool arriba  = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
                    || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        bool abajo   = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
                    || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        bool enter   = sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                    || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);

        if (arriba && !arribaPress && tiempoApertura > 0.15f)
            opcion = (opcion - 1 + NUM_OPCIONES) % NUM_OPCIONES;
        if (abajo && !abajoPress && tiempoApertura > 0.15f)
            opcion = (opcion + 1) % NUM_OPCIONES;

        ResultadoPausa res = ResultadoPausa::Ninguno;

        if (enter && !enterPress && tiempoApertura > 0.15f) {
            switch (opcion) {
                case 0: cerrar(); res = ResultadoPausa::Continuar;   break;
                case 1: // Música
                    musicaMuteada = !musicaMuteada;
                    musica.setVolume(musicaMuteada ? 0.f : 20.f);
                    break;
                case 2: // Fullscreen
                    pantallaCompleta = !pantallaCompleta;
                    if (pantallaCompleta)
                        ventana.create(sf::VideoMode::getDesktopMode(),
                                       "Overdrive", sf::Style::Fullscreen);
                    else
                        ventana.create(sf::VideoMode(1280, 720),
                                       "Overdrive", sf::Style::Close | sf::Style::Titlebar);
                    break;
                case 3: cerrar(); res = ResultadoPausa::VolverMenu;  break;
            }
        }

        if (esc && !escPress && tiempoApertura > 0.15f) {
            cerrar();
            res = ResultadoPausa::Continuar;
        }

        arribaPress = arriba;
        abajoPress  = abajo;
        enterPress  = enter;
        escPress    = esc;
        return res;
    }

    void draw(bool musicaMuteada, bool pantallaCompleta) {
        if (!activo) return;

        float W = (float)ventana.getSize().x;
        float H = (float)ventana.getSize().y;

        // ── Overlay oscuro animado (aparece progresivamente) ──
        float fade = std::min(1.f, tiempoApertura / 0.2f);
        sf::RectangleShape overlay({W, H});
        overlay.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(160 * fade)));
        ventana.draw(overlay);

        // ── Panel central ─────────────────────────────────────
        float pw = 400.f, ph = 340.f;
        float px = (W - pw) * 0.5f;
        float py = (H - ph) * 0.5f;

        float escPanel = 0.85f + 0.15f * std::min(1.f, tiempoApertura / 0.18f);
        float pwS = pw * escPanel, phS = ph * escPanel;
        float pxS = (W - pwS) * 0.5f, pyS = (H - phS) * 0.5f;

        // Fondo
        sf::RectangleShape panel({pwS, phS});
        panel.setPosition(pxS, pyS);
        panel.setFillColor(sf::Color(8, 10, 24, 230));
        panel.setOutlineColor(sf::Color(0, 200, 255, 200));
        panel.setOutlineThickness(2.f);
        ventana.draw(panel);

        // Línea superior neón
        sf::RectangleShape linSup({pwS - 4, 3.f});
        linSup.setPosition(pxS + 2, pyS + 2);
        linSup.setFillColor(sf::Color(0, 220, 255));
        ventana.draw(linSup);

        // ── Título PAUSA ───────────────────────────────────────
        float pulso = 0.5f + 0.5f * std::sin(tiempoApertura * 3.5f);
        sf::Text titulo;
        titulo.setFont(fuente);
        titulo.setCharacterSize(42);
        titulo.setFillColor(sf::Color(0, (sf::Uint8)(180 + 75 * pulso), 255));
        titulo.setString("PAUSA");
        titulo.setLetterSpacing(4.f);
        sf::FloatRect b = titulo.getLocalBounds();
        titulo.setOrigin(b.left + b.width * 0.5f, b.top);
        titulo.setPosition(W * 0.5f, pyS + 18.f);
        ventana.draw(titulo);

        // Línea bajo título
        sf::RectangleShape linTit({pwS - 40.f, 1.5f});
        linTit.setPosition(pxS + 20, pyS + 68);
        linTit.setFillColor(sf::Color(0, 180, 255, 150));
        ventana.draw(linTit);

        // ── Opciones ──────────────────────────────────────────
        struct Opcion { std::string label; std::string valor; };
        Opcion opciones[NUM_OPCIONES] = {
            {"CONTINUAR",        ""},
            {"MUSICA",           musicaMuteada    ? "MUTED" : "ON"},
            {"PANTALLA COMPLETA",pantallaCompleta ? "ON"    : "OFF"},
            {"VOLVER AL MENU",   ""},
        };

        float startY = pyS + 82.f;
        float step   = 55.f;

        for (int i = 0; i < NUM_OPCIONES; ++i) {
            bool sel = (i == opcion);
            float y  = startY + i * step;

            // Fondo de opción seleccionada
            if (sel) {
                sf::RectangleShape hl({pwS - 40.f, 42.f});
                hl.setPosition(pxS + 20, y - 4);
                hl.setFillColor(sf::Color(0, 180, 255, 25));
                hl.setOutlineColor(sf::Color(0, 200, 255, 180));
                hl.setOutlineThickness(1.f);
                ventana.draw(hl);

                // Acento lateral
                sf::RectangleShape acento({4.f, 42.f});
                acento.setPosition(pxS + 20, y - 4);
                acento.setFillColor(sf::Color(0, 220, 255));
                ventana.draw(acento);
            }

            // Texto de la opción
            sf::Text txt;
            txt.setFont(fuente);
            txt.setCharacterSize(20);
            txt.setFillColor(sel ? sf::Color(0, 220, 255) : sf::Color(160, 170, 200));
            txt.setString(opciones[i].label);
            txt.setPosition(pxS + 36.f, y + 4.f);
            ventana.draw(txt);

            // Valor a la derecha (para toggles)
            if (!opciones[i].valor.empty()) {
                sf::Text val;
                val.setFont(fuente);
                val.setCharacterSize(18);
                bool on = (opciones[i].valor == "ON");
                val.setFillColor(on ? sf::Color(80, 255, 140) : sf::Color(120, 120, 150));
                val.setString(opciones[i].valor);
                sf::FloatRect vb = val.getLocalBounds();
                val.setOrigin(vb.left + vb.width, vb.top);
                val.setPosition(pxS + pwS - 28.f, y + 6.f);
                ventana.draw(val);
            }
        }

        // ── Pie ─────────────────────────────────────────────
        sf::Text pie;
        pie.setFont(fuente);
        pie.setCharacterSize(13);
        pie.setFillColor(sf::Color(70, 80, 110));
        pie.setString("W/S NAVEGAR  |  ENTER SELECCIONAR  |  ESC CONTINUAR");
        sf::FloatRect pb = pie.getLocalBounds();
        pie.setOrigin(pb.left + pb.width * 0.5f, pb.top);
        pie.setPosition(W * 0.5f, pyS + phS - 26.f);
        ventana.draw(pie);
    }

private:
    sf::RenderWindow& ventana;
    sf::Music&        musica;
    sf::Font&         fuente;

    bool  activo         = false;
    int   opcion         = 0;
    float tiempoApertura = 0.f;

    bool arribaPress = false;
    bool abajoPress  = false;
    bool enterPress  = false;
    bool escPress    = false;

    static constexpr int NUM_OPCIONES = 4;
};
