#pragma once
#include <SFML/Window.hpp>
#include "Constantes.hpp"

// ============================================================
// Input.hpp — Overdrive
// Abstracción de entrada unificada: teclado + gamepad.
// Cada jugador tiene su propio InputHandler.
// Si hay gamepad conectado, tiene prioridad sobre teclado.
// Compatible con Xbox y PlayStation (SFML los mapea igual).
// ============================================================

enum class Accion {
    Izquierda,
    Derecha,
    Saltar,
    Deslizar,
    Gancho,   // LB / RB / LShift / RShift
    Pausa
};

class InputHandler {
public:
    // idJugador: 0 o 1
    // idGamepad:  índice del joystick SFML (0 = primer gamepad, 1 = segundo)
    explicit InputHandler(int idJugador)
        : idJug(idJugador), idJoy(idJugador)
    {
        // Teclado por defecto
        if (idJugador == 0) {
            kIzq     = sf::Keyboard::A;
            kDer     = sf::Keyboard::D;
            kSaltar  = sf::Keyboard::W;
            kDesliz  = sf::Keyboard::S;
            kGancho  = sf::Keyboard::LShift;
            kPausa   = sf::Keyboard::Escape;
        } else {
            kIzq     = sf::Keyboard::Left;
            kDer     = sf::Keyboard::Right;
            kSaltar  = sf::Keyboard::Up;
            kDesliz  = sf::Keyboard::Down;
            kGancho  = sf::Keyboard::RShift;
            kPausa   = sf::Keyboard::Escape;
        }
    }

    // Llamar una vez por frame para cachear estado
    void update() {
        joyConectado = sf::Joystick::isConnected(idJoy);
        if (joyConectado)
            sf::Joystick::update();
    }

    bool presionado(Accion a) const {
        return joyConectado ? joyPresionado(a) : teclaPresionada(a);
    }

    // Retorna el eje horizontal [-1, 1]
    float ejeX() const {
        if (!joyConectado) {
            float v = 0.f;
            if (sf::Keyboard::isKeyPressed(kDer))  v += 1.f;
            if (sf::Keyboard::isKeyPressed(kIzq))  v -= 1.f;
            return v;
        }
        float raw = sf::Joystick::getAxisPosition(idJoy, sf::Joystick::X);
        if (std::abs(raw) < STICK_DEAD) return 0.f;
        return raw / 100.f;
    }

    bool hayGamepad() const { return joyConectado; }

private:
    int  idJug, idJoy;
    bool joyConectado = false;

    // Teclas asignadas
    sf::Keyboard::Key kIzq, kDer, kSaltar, kDesliz, kGancho, kPausa;

    bool teclaPresionada(Accion a) const {
        switch (a) {
            case Accion::Izquierda: return sf::Keyboard::isKeyPressed(kIzq);
            case Accion::Derecha:   return sf::Keyboard::isKeyPressed(kDer);
            case Accion::Saltar:    return sf::Keyboard::isKeyPressed(kSaltar);
            case Accion::Deslizar:  return sf::Keyboard::isKeyPressed(kDesliz);
            case Accion::Gancho:    return sf::Keyboard::isKeyPressed(kGancho);
            case Accion::Pausa:     return sf::Keyboard::isKeyPressed(kPausa);
            default: return false;
        }
    }

    bool joyPresionado(Accion a) const {
        switch (a) {
            case Accion::Izquierda:
                return sf::Joystick::getAxisPosition(idJoy, sf::Joystick::X) < -STICK_DEAD;
            case Accion::Derecha:
                return sf::Joystick::getAxisPosition(idJoy, sf::Joystick::X) >  STICK_DEAD;
            case Accion::Saltar:
                return sf::Joystick::isButtonPressed(idJoy, BTN_A);
            case Accion::Deslizar:
                return sf::Joystick::isButtonPressed(idJoy, BTN_B)
                    || sf::Joystick::getAxisPosition(idJoy, sf::Joystick::Y) > STICK_DEAD;
            case Accion::Gancho:
                return sf::Joystick::isButtonPressed(idJoy, BTN_LB)
                    || sf::Joystick::isButtonPressed(idJoy, BTN_RB);
            case Accion::Pausa:
                return sf::Joystick::isButtonPressed(idJoy, BTN_START);
            default: return false;
        }
    }
};
