#pragma once
// ============================================================
// Countdown.hpp — Overdrive
// Contador animado 3-2-1-GO! Se dibuja sobre el juego.
// ============================================================
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>

class Countdown {
public:
    explicit Countdown(sf::RenderWindow& v) : ventana(v) {
        if (!fuente.loadFromFile("assets/fonts/Rajdhani-Bold.ttf"))
            fuente.loadFromFile("assets/fonts/Ring.ttf");
    }

    void iniciar() { timer = 0.f; termino = false; }
    bool haTerminado() const { return termino; }

    bool update(float dt) {
        if (termino) return false;
        timer += dt;
        if (timer >= DURACION_TOTAL) { termino = true; return false; }
        return true;
    }

    void draw() {
        if (termino) return;
        float W = (float)ventana.getSize().x;
        float H = (float)ventana.getSize().y;

        // Overlay
        sf::RectangleShape ov({W,H});
        ov.setFillColor(sf::Color(0,0,0,100));
        ventana.draw(ov);

        std::string label; sf::Color color; float localT, beatDur;
        if      (timer < 1.f) { label="3"; color=sf::Color(255,80,80);  localT=timer;     beatDur=1.f; }
        else if (timer < 2.f) { label="2"; color=sf::Color(255,160,40); localT=timer-1.f; beatDur=1.f; }
        else if (timer < 3.f) { label="1"; color=sf::Color(80,220,80);  localT=timer-2.f; beatDur=1.f; }
        else                   { label="GO!";color=sf::Color(0,220,255); localT=timer-3.f; beatDur=0.5f;}

        float t = localT / beatDur;
        float escala, alpha;
        if (label=="GO!") {
            escala = 0.5f + 0.9f*t;
            alpha  = (t<0.7f) ? 1.f : 1.f-(t-0.7f)/0.3f;
        } else {
            float bounce = std::sin(t*3.14159f)*0.25f;
            escala = 1.8f - 0.9f*t + bounce;
            alpha  = (t<0.8f) ? 1.f : 1.f-(t-0.8f)/0.2f;
        }
        sf::Uint8 a = (sf::Uint8)(std::max(0.f,std::min(1.f,alpha))*255.f);

        unsigned cs = (unsigned)(130.f*escala);
        if (cs < 10) cs=10;

        auto makeText = [&](const std::string& s, sf::Color c, float ox=0, float oy=0){
            sf::Text tx; tx.setFont(fuente); tx.setCharacterSize(cs);
            tx.setFillColor(c); tx.setString(s);
            sf::FloatRect b=tx.getLocalBounds();
            tx.setOrigin(b.left+b.width*0.5f, b.top+b.height*0.5f);
            tx.setPosition(W*0.5f+ox, H*0.5f+oy);
            return tx;
        };

        // Sombra
        for (int d=-3;d<=3;d+=3) {
            sf::Color sh(0,0,0,(sf::Uint8)(a*0.5f));
            ventana.draw(makeText(label,sh,(float)d*2,(float)d*2));
        }
        // Halo
        sf::Color halo(color.r,color.g,color.b,(sf::Uint8)(a*0.35f));
        for (int dx=-4;dx<=4;dx+=2) for(int dy=-4;dy<=4;dy+=2) {
            if(dx==0&&dy==0) continue;
            ventana.draw(makeText(label,halo,(float)dx,(float)dy));
        }
        // Principal
        sf::Text principal=makeText(label,sf::Color(color.r,color.g,color.b,a));
        principal.setOutlineColor(sf::Color(0,0,0,a));
        principal.setOutlineThickness(3.f);
        ventana.draw(principal);

        // Líneas laterales
        float lineW = 200.f*std::min(1.f,t*3.f);
        float lineY = H*0.5f;
        sf::Color lc(color.r,color.g,color.b,(sf::Uint8)(a*0.7f));
        sf::RectangleShape lI({lineW,3.f}), lD({lineW,3.f});
        lI.setPosition(W*0.5f-80.f-lineW, lineY-1.5f); lI.setFillColor(lc);
        lD.setPosition(W*0.5f+80.f,        lineY-1.5f); lD.setFillColor(lc);
        ventana.draw(lI); ventana.draw(lD);
    }

private:
    sf::RenderWindow& ventana;
    sf::Font          fuente;
    float             timer   = 0.f;
    bool              termino = false;
    static constexpr float DURACION_TOTAL = 3.5f;
};
