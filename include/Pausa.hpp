#pragma once
// ============================================================
// Pausa.hpp — Overdrive  v2
// Menú de pausa in-game.
// - Se abre con ESC (evento KeyPressed desde main)
// - Opciones: CONTINUAR / MÚSICA / VOLVER AL MENÚ
// - Usa eventos discretos (no polling) para evitar el bug
//   de que ESC abra y cierre la pausa en el mismo frame.
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <string>

enum class ResultadoPausa { Ninguno, Continuar, VolverMenu };

class Pausa {
public:
    Pausa(sf::RenderWindow& v, sf::Music& m, sf::Font& f)
        : ventana(v), musica(m), fuente(f) {}

    bool estaActivo() const { return activo; }

    void abrir() {
        activo     = true;
        opcion     = 0;
        tApertura  = 0.f;
        // Bajar volumen al pausar (si no está muteado)
        if (!muted) musica.setVolume(20.f);
    }

    void cerrar() {
        activo = false;
        // Restaurar volumen al cerrar
        if (!muted) musica.setVolume(50.f);
    }

    // Procesa un único evento de teclado. Llamar desde el pollEvent de main.
    // Devuelve la acción resultante.
    ResultadoPausa procesarEvento(const sf::Event& ev, bool& musicaMuteada) {
        if (!activo) return ResultadoPausa::Ninguno;
        if (ev.type != sf::Event::KeyPressed) return ResultadoPausa::Ninguno;

        // Bloqueo mínimo de apertura para ignorar el ESC que la abrió
        if (tApertura < 0.1f) return ResultadoPausa::Ninguno;

        switch (ev.key.code) {
            case sf::Keyboard::Up:
            case sf::Keyboard::W:
                opcion = (opcion - 1 + N) % N;
                break;

            case sf::Keyboard::Down:
            case sf::Keyboard::S:
                opcion = (opcion + 1) % N;
                break;

            case sf::Keyboard::Return:
            case sf::Keyboard::Space:
                if (opcion == 0) {
                    cerrar();
                    return ResultadoPausa::Continuar;
                } else if (opcion == 1) {
                    // Toggle música
                    musicaMuteada = !musicaMuteada;
                    muted         = musicaMuteada;
                    musica.setVolume(musicaMuteada ? 0.f : 20.f);
                } else if (opcion == 2) {
                    cerrar();
                    return ResultadoPausa::VolverMenu;
                }
                break;

            case sf::Keyboard::Escape:
                cerrar();
                return ResultadoPausa::Continuar;

            default: break;
        }
        return ResultadoPausa::Ninguno;
    }

    // Avanza el timer de apertura (para el bloqueo anti-rebote)
    void update(float dt) {
        if (activo) tApertura += dt;
    }

    void draw(bool musicaMuteada) {
        if (!activo) return;
        float W = (float)ventana.getSize().x;
        float H = (float)ventana.getSize().y;

        // Overlay oscuro con fade-in
        float fade = std::min(1.f, tApertura / 0.2f);
        sf::RectangleShape ov({W, H});
        ov.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(160 * fade)));
        ventana.draw(ov);

        // Panel con animación de escala
        float pw = 420.f, ph = 295.f;
        float sc = 0.85f + 0.15f * std::min(1.f, tApertura / 0.18f);
        float pws = pw * sc, phs = ph * sc;
        float pxs = (W - pws) * 0.5f, pys = (H - phs) * 0.5f;

        sf::RectangleShape panel({pws, phs});
        panel.setPosition(pxs, pys);
        panel.setFillColor(sf::Color(8, 10, 24, 235));
        panel.setOutlineColor(sf::Color(0, 200, 255, 200));
        panel.setOutlineThickness(2.f);
        ventana.draw(panel);

        // Barra superior neón
        sf::RectangleShape bar({pws - 4, 3.f});
        bar.setPosition(pxs + 2, pys + 2);
        bar.setFillColor(sf::Color(0, 220, 255));
        ventana.draw(bar);

        // Título
        float pulso = 0.5f + 0.5f * std::sin(tApertura * 3.5f);
        sf::Text titulo;
        titulo.setFont(fuente);
        titulo.setCharacterSize(44);
        titulo.setFillColor(sf::Color(0, (sf::Uint8)(180 + 75 * pulso), 255));
        titulo.setString("PAUSA");
        titulo.setLetterSpacing(4.f);
        sf::FloatRect tb = titulo.getLocalBounds();
        titulo.setOrigin(tb.left + tb.width * 0.5f, tb.top);
        titulo.setPosition(W * 0.5f, pys + 14.f);
        ventana.draw(titulo);

        sf::RectangleShape sep({pws - 40.f, 1.5f});
        sep.setPosition(pxs + 20, pys + 66);
        sep.setFillColor(sf::Color(0, 180, 255, 150));
        ventana.draw(sep);

        // Opciones
        struct OpInfo { std::string label; std::string valor; };
        OpInfo ops[N] = {
            {"CONTINUAR",      ""},
            {"MUSICA",         musicaMuteada ? "MUTED" : "ON"},
            {"VOLVER AL MENU", ""},
        };

        float startY = pys + 82.f, step = 56.f;
        for (int i = 0; i < N; ++i) {
            bool sel = (i == opcion);
            float y = startY + i * step;

            if (sel) {
                sf::RectangleShape hl({pws - 40.f, 42.f});
                hl.setPosition(pxs + 20, y - 4);
                hl.setFillColor(sf::Color(0, 180, 255, 25));
                hl.setOutlineColor(sf::Color(0, 200, 255, 180));
                hl.setOutlineThickness(1.f);
                ventana.draw(hl);

                sf::RectangleShape ac({4.f, 42.f});
                ac.setPosition(pxs + 20, y - 4);
                ac.setFillColor(sf::Color(0, 220, 255));
                ventana.draw(ac);
            }

            sf::Text tx;
            tx.setFont(fuente);
            tx.setCharacterSize(21);
            tx.setFillColor(sel ? sf::Color(0, 220, 255) : sf::Color(160, 170, 200));
            tx.setString(ops[i].label);
            tx.setPosition(pxs + 36.f, y + 4.f);
            ventana.draw(tx);

            if (!ops[i].valor.empty()) {
                sf::Text val;
                val.setFont(fuente);
                val.setCharacterSize(19);
                bool on = (ops[i].valor == "ON");
                val.setFillColor(on ? sf::Color(80, 255, 140) : sf::Color(120, 120, 150));
                val.setString(ops[i].valor);
                sf::FloatRect vb = val.getLocalBounds();
                val.setOrigin(vb.left + vb.width, vb.top);
                val.setPosition(pxs + pws - 28.f, y + 6.f);
                ventana.draw(val);
            }
        }

        // Pie
        sf::Text pie;
        pie.setFont(fuente);
        pie.setCharacterSize(13);
        pie.setFillColor(sf::Color(70, 80, 110));
        pie.setString("W/S  NAVEGAR    ENTER  SELECCIONAR    ESC  CONTINUAR");
        sf::FloatRect pb = pie.getLocalBounds();
        pie.setOrigin(pb.left + pb.width * 0.5f, pb.top);
        pie.setPosition(W * 0.5f, pys + phs - 22.f);
        ventana.draw(pie);
    }

private:
    sf::RenderWindow& ventana;
    sf::Music&        musica;
    sf::Font&         fuente;

    bool  activo    = false;
    bool  muted     = false;
    int   opcion    = 0;
    float tApertura = 0.f;

    static constexpr int N = 3;
};
