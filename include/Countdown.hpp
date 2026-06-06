#pragma once
// ============================================================
// Countdown.hpp — Overdrive
// Contador animado 3-2-1-¡GO! que bloquea la partida
// mientras se dibuja sobre el juego ya renderizado.
// ============================================================
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>

class Countdown {
public:
    // duracionTotal: cuántos segundos dura todo el conteo (ej. 3.5)
    // La secuencia es:  3 (1s) → 2 (1s) → 1 (1s) → GO (0.5s) → fin
    explicit Countdown(sf::RenderWindow& v)
        : ventana(v)
    {
        if (!fuente.loadFromFile("assets/fonts/Rajdhani-Bold.ttf"))
            fuente.loadFromFile("assets/fonts/Ring.ttf");
    }

    // Reinicia el contador (llámalo justo antes de spawn)
    void iniciar() {
        timer    = 0.f;
        terminó  = false;
    }

    bool haTerminado() const { return terminó; }

    // Avanza el timer. Devuelve true mientras sigue activo.
    bool update(float dt) {
        if (terminó) return false;
        timer += dt;
        if (timer >= DURACION_TOTAL) {
            terminó = true;
            return false;
        }
        return true;
    }

    // Dibuja el número actual sobre lo que ya se haya renderizado.
    // Llámalo DESPUÉS del render del juego, ANTES de display().
    void draw() {
        if (terminó) return;

        float W = (float)ventana.getSize().x;
        float H = (float)ventana.getSize().y;

        // ── Overlay semitransparente ──────────────────────────
        sf::RectangleShape overlay({W, H});
        overlay.setFillColor(sf::Color(0, 0, 0, 100));
        ventana.draw(overlay);

        // ── Qué número mostrar ────────────────────────────────
        std::string label;
        sf::Color   color;
        float       localT;   // tiempo dentro de este "beat" (0..1s o 0..0.5s)
        float       beatDur;

        if (timer < 1.f) {
            label = "3";  color = sf::Color(255, 80,  80);
            localT = timer;       beatDur = 1.f;
        } else if (timer < 2.f) {
            label = "2";  color = sf::Color(255, 160, 40);
            localT = timer - 1.f; beatDur = 1.f;
        } else if (timer < 3.f) {
            label = "1";  color = sf::Color(80,  220, 80);
            localT = timer - 2.f; beatDur = 1.f;
        } else {
            label = "GO!"; color = sf::Color(0,  220, 255);
            localT = timer - 3.f; beatDur = 0.5f;
        }

        float t = localT / beatDur;  // 0..1 normalizado

        // ── Animación: entra grande y se encoge / desvanece ───
        // Escala: empieza en 2.0 → decrece a 0.8, luego en GO sube a 1.5
        float escala, alpha;
        if (label == "GO!") {
            // GO: crece de 0.5 → 1.4, se desvanece al final
            escala = 0.5f + 0.9f * t;
            alpha  = (t < 0.7f) ? 1.f : 1.f - (t - 0.7f) / 0.3f;
        } else {
            // Números: caen de grande a pequeño con bounce leve
            float bounce = std::sin(t * 3.14159f) * 0.25f;
            escala = 1.8f - 0.9f * t + bounce;
            alpha  = (t < 0.8f) ? 1.f : 1.f - (t - 0.8f) / 0.2f;
        }

        sf::Uint8 a = (sf::Uint8)(std::max(0.f, std::min(1.f, alpha)) * 255.f);

        // ── Sombra / halo ──────────────────────────────────────
        float sz = 130.f * escala;
        auto makeText = [&](const std::string& s, unsigned cs,
                             sf::Color c, float ox=0, float oy=0) {
            sf::Text tx;
            tx.setFont(fuente);
            tx.setCharacterSize(cs);
            tx.setFillColor(c);
            tx.setString(s);
            sf::FloatRect b = tx.getLocalBounds();
            tx.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
            tx.setPosition(W * 0.5f + ox, H * 0.5f + oy);
            return tx;
        };

        unsigned cs = (unsigned)(130.f * escala);
        if (cs < 10) cs = 10;

        // Sombra difusa
        sf::Color sombra(0, 0, 0, (sf::Uint8)(a * 0.5f));
        for (int d = -3; d <= 3; d += 3) {
            sf::Text sh = makeText(label, cs, sombra, (float)d * 2, (float)d * 2);
            ventana.draw(sh);
        }

        // Halo de color
        sf::Color halo(color.r, color.g, color.b, (sf::Uint8)(a * 0.35f));
        for (int dx = -4; dx <= 4; dx += 2) {
            for (int dy = -4; dy <= 4; dy += 2) {
                if (dx == 0 && dy == 0) continue;
                sf::Text h = makeText(label, cs, halo, (float)dx, (float)dy);
                ventana.draw(h);
            }
        }

        // Texto principal
        sf::Color colFinal(color.r, color.g, color.b, a);
        sf::Text principal = makeText(label, cs, colFinal);
        principal.setOutlineColor(sf::Color(0, 0, 0, a));
        principal.setOutlineThickness(3.f);
        ventana.draw(principal);

        // ── Líneas laterales decorativas ──────────────────────
        float lineW = 200.f * std::min(1.f, t * 3.f);
        float lineY = H * 0.5f;
        sf::Color lc(color.r, color.g, color.b, (sf::Uint8)(a * 0.7f));

        // Izquierda
        sf::RectangleShape lI({lineW, 3.f});
        lI.setPosition(W * 0.5f - 80.f - lineW, lineY - 1.5f);
        lI.setFillColor(lc);
        ventana.draw(lI);

        // Derecha
        sf::RectangleShape lD({lineW, 3.f});
        lD.setPosition(W * 0.5f + 80.f, lineY - 1.5f);
        lD.setFillColor(lc);
        ventana.draw(lD);
    }

private:
    sf::RenderWindow& ventana;
    sf::Font          fuente;
    float             timer        = 0.f;
    bool              terminó      = false;

    static constexpr float DURACION_TOTAL = 3.5f;  // 3+2+1+GO
};
