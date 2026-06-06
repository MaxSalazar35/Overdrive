#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

// ============================================================
// Particulas.hpp — Overdrive
// Sistema de partículas para:
//  - Polvo al correr / aterrizar
//  - Chispas al usar gancho
//  - Explosión de confetti al ganar ronda
//  - Estela de velocidad
// ============================================================

struct Particula {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color    color;
    float        vida;        // segundos restantes
    float        vidaMax;
    float        tamInicio;
    float        tamFin;
    float        rotacion;
    float        velRot;
    bool         activa;
};

enum class TipoParticula {
    Polvo,
    Chispa,
    Confetti,
    Estela
};

class SistemaParticulas {
public:
    SistemaParticulas() { particulas.reserve(500); }

    // Emitir un burst en una posición
    void emitir(sf::Vector2f pos, TipoParticula tipo, sf::Color color = sf::Color::White, int cantidad = 12) {
        for (int i = 0; i < cantidad; ++i) {
            Particula p;
            p.pos      = pos;
            p.activa   = true;
            p.rotacion = 0.f;

            switch (tipo) {
                case TipoParticula::Polvo:
                    p.vel      = aleatorio(-60, 60, -120, -20);
                    p.vida     = p.vidaMax = 0.35f + aleatoriof() * 0.2f;
                    p.tamInicio= 8.f + aleatoriof() * 6.f;
                    p.tamFin   = 0.f;
                    p.color    = sf::Color(200, 190, 170, 200);
                    p.velRot   = 0.f;
                    break;

                case TipoParticula::Chispa:
                    p.vel      = aleatorio(-200, 200, -250, -50);
                    p.vida     = p.vidaMax = 0.25f + aleatoriof() * 0.15f;
                    p.tamInicio= 4.f;
                    p.tamFin   = 0.f;
                    p.color    = sf::Color(255, 220, 80, 255);
                    p.velRot   = 0.f;
                    break;

                case TipoParticula::Confetti:
                    p.vel      = aleatorio(-350, 350, -500, -100);
                    p.vida     = p.vidaMax = 1.2f + aleatoriof() * 0.8f;
                    p.tamInicio= 10.f + aleatoriof() * 8.f;
                    p.tamFin   = p.tamInicio;
                    p.color    = colorAleatorio();
                    p.velRot   = (aleatoriof() - 0.5f) * 720.f;
                    break;

                case TipoParticula::Estela:
                    p.vel      = sf::Vector2f((aleatoriof() - 0.5f) * 40.f, (aleatoriof() - 0.5f) * 20.f);
                    p.vida     = p.vidaMax = 0.15f + aleatoriof() * 0.1f;
                    p.tamInicio= 6.f;
                    p.tamFin   = 0.f;
                    p.color    = color;
                    p.velRot   = 0.f;
                    break;
            }

            // Buscar slot libre
            bool colocada = false;
            for (auto& slot : particulas) {
                if (!slot.activa) { slot = p; colocada = true; break; }
            }
            if (!colocada && particulas.size() < 500)
                particulas.push_back(p);
        }
    }

    void update(float dt) {
        for (auto& p : particulas) {
            if (!p.activa) continue;
            p.vida     -= dt;
            if (p.vida <= 0.f) { p.activa = false; continue; }
            p.pos      += p.vel * dt;
            p.vel.y    += 400.f * dt;  // gravedad leve
            p.rotacion += p.velRot * dt;
            // Fade alpha
            float t   = p.vida / p.vidaMax;
            p.color.a = static_cast<sf::Uint8>(255.f * t);
        }
    }

    void draw(sf::RenderWindow& ventana) {
        for (const auto& p : particulas) {
            if (!p.activa) continue;
            float t   = p.vida / p.vidaMax;
            float tam = p.tamInicio * t + p.tamFin * (1.f - t);
            if (tam < 0.5f) continue;

            sf::RectangleShape rect(sf::Vector2f(tam, tam));
            rect.setOrigin(tam / 2.f, tam / 2.f);
            rect.setPosition(p.pos);
            rect.setFillColor(p.color);
            rect.setRotation(p.rotacion);
            ventana.draw(rect);
        }
    }

    void limpiar() {
        for (auto& p : particulas) p.activa = false;
    }

private:
    std::vector<Particula> particulas;

    float aleatoriof() const {
        return static_cast<float>(std::rand()) / RAND_MAX;
    }

    sf::Vector2f aleatorio(float vxMin, float vxMax, float vyMin, float vyMax) const {
        return {
            vxMin + aleatoriof() * (vxMax - vxMin),
            vyMin + aleatoriof() * (vyMax - vyMin)
        };
    }

    sf::Color colorAleatorio() const {
        static const sf::Color colores[] = {
            sf::Color(255, 80,  80),
            sf::Color(80,  180, 255),
            sf::Color(80,  255, 120),
            sf::Color(255, 220, 60),
            sf::Color(220, 80,  255),
            sf::Color(255, 140, 40)
        };
        return colores[std::rand() % 6];
    }
};
