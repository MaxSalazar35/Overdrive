#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <cmath>

// ============================================================
// Animacion.hpp — componente de animacion por spritesheet
// ============================================================

struct FrameAnim {
    int   fila;
    int   totalFrames;
    float tiempoFrame;  // segundos por frame
    bool  loop;
};

class Animacion {
public:
    void configurar(int frameW, int frameH) { fw = frameW; fh = frameH; }

    void agregarEstado(int id, FrameAnim cfg) { estados[id] = cfg; }

    // Cambia de estado. Si forceReset=true reinicia aunque sea el mismo estado.
    void setEstado(int id, bool forceReset = false) {
        if (estadoActual == id && !forceReset) return;
        estadoActual = id;
        frameActual  = 0;
        terminado    = false;
        reloj.restart();
    }

    // Congela la animacion en el ultimo frame sin cambiar el estado logico.
    // Util para mostrar el frame final de caida o desliz-salida.
    void congelarUltimoFrame() {
        auto it = estados.find(estadoActual);
        if (it == estados.end()) return;
        frameActual = it->second.totalFrames - 1;
        terminado   = true;
    }

    // Fuerza un frame especifico (0-based)
    void setFrame(int f) {
        auto it = estados.find(estadoActual);
        if (it == estados.end()) return;
        frameActual = std::min(f, it->second.totalFrames - 1);
    }

    int getFrameActual() const { return frameActual; }
    int getTotalFrames() const {
        auto it = estados.find(estadoActual);
        return it != estados.end() ? it->second.totalFrames : 1;
    }

    void update() {
        auto it = estados.find(estadoActual);
        if (it == estados.end()) return;
        const FrameAnim& cfg = it->second;
        if (reloj.getElapsedTime().asSeconds() >= cfg.tiempoFrame) {
            reloj.restart();
            frameActual++;
            if (frameActual >= cfg.totalFrames) {
                frameActual = cfg.loop ? 0 : cfg.totalFrames - 1;
                terminado   = !cfg.loop;
            }
        }
    }

    sf::IntRect getRect() const {
        int fila = 0;
        auto it = estados.find(estadoActual);
        if (it != estados.end()) fila = it->second.fila;
        return sf::IntRect(frameActual * fw, fila * fh, fw, fh);
    }

    bool estaTerminado() const { return terminado; }

private:
    int  fw = 200, fh = 220;
    int  estadoActual = 0;
    int  frameActual  = 0;
    bool terminado    = false;
    sf::Clock reloj;
    std::map<int, FrameAnim> estados;
};
