#pragma once
// ============================================================
// Audio.hpp — Overdrive
// Sistema de audio centralizado:
//  - Música alterna: musica.ogg ↔ musica2.ogg en loop
//  - victorysong.ogg: solo cuando alguien gana el juego final
//  - Efectos (SoundBuffer): opcion, death, cajas
// ============================================================
#include <SFML/Audio.hpp>
#include <string>

class SistemaAudio {
public:
    SistemaAudio() {
        // ── Música principal (dos pistas que se alternan) ────
        pista1OK = musica1.openFromFile("assets/music/musica.ogg");
        pista2OK = musica2.openFromFile("assets/sounds/musica2.ogg");

        if (pista1OK) {
            musica1.setLoop(false);   // no loop — alternamos manualmente
            musica1.setVolume(50.f);
        }
        if (pista2OK) {
            musica2.setLoop(false);
            musica2.setVolume(50.f);
        }

        // ── Victoria ─────────────────────────────────────────
        victoryOK = victory.openFromFile("assets/sounds/victorysong.ogg");
        if (victoryOK) {
            victory.setLoop(false);
            victory.setVolume(70.f);
        }

        // ── Efectos de sonido ────────────────────────────────
        bufOpcionOK = bufOpcion.loadFromFile("assets/sounds/opcion.ogg");
        bufDeathOK  = bufDeath.loadFromFile("assets/sounds/death.ogg");
        bufCajaOK   = bufCaja.loadFromFile("assets/sounds/cajas.ogg");

        if (bufOpcionOK) { sndOpcion.setBuffer(bufOpcion); sndOpcion.setVolume(70.f); }
        if (bufDeathOK)  { sndDeath.setBuffer(bufDeath);   sndDeath.setVolume(80.f);  }
        if (bufCajaOK)   { sndCaja.setBuffer(bufCaja);     sndCaja.setVolume(65.f);   }
    }

    // ── Música principal ──────────────────────────────────────
    void iniciarMusica() {
        victory.stop();
        muted = false;
        if (pista1OK) { musica1.play(); pistActual = 1; }
        else if (pista2OK) { musica2.play(); pistActual = 2; }
    }

    // Llamar cada frame: alterna pistas cuando una termina
    void updateMusica() {
        if (muted || enVictoria) return;
        if (pistActual == 1) {
            if (musica1.getStatus() == sf::Music::Stopped) {
                if (pista2OK) { musica2.play(); pistActual = 2; }
                else if (pista1OK) { musica1.play(); }
            }
        } else {
            if (musica2.getStatus() == sf::Music::Stopped) {
                if (pista1OK) { musica1.play(); pistActual = 1; }
                else if (pista2OK) { musica2.play(); }
            }
        }
    }

    // ── Victoria ──────────────────────────────────────────────
    void iniciarVictoria() {
        musica1.pause();
        musica2.pause();
        enVictoria = true;
        if (victoryOK && !muted) victory.play();
    }

    void detenerVictoria() {
        victory.stop();
        enVictoria = false;
        // Retomar la música donde se quedó
        if (!muted) {
            if (pistActual == 1 && pista1OK) musica1.play();
            else if (pista2OK) musica2.play();
        }
    }

    // ── Mute/unmute (afecta todo) ─────────────────────────────
    void setMuted(bool m) {
        muted = m;
        float vol = m ? 0.f : 50.f;
        musica1.setVolume(vol);
        musica2.setVolume(vol);
        victory.setVolume(m ? 0.f : 70.f);
    }
    bool isMuted() const { return muted; }

    // Referencia a la música activa (para pasarla a Menu/Pausa)
    sf::Music& getMusicaRef() {
        return (pistActual == 2 && pista2OK) ? musica2 : musica1;
    }

    // ── Efectos ───────────────────────────────────────────────
    void playOpcion() { if (bufOpcionOK && !muted) sndOpcion.play(); }
    void playDeath()  { if (bufDeathOK  && !muted) sndDeath.play();  }
    void playCaja()   { if (bufCajaOK   && !muted) sndCaja.play();   }

private:
    // Música alterna
    sf::Music musica1, musica2, victory;
    bool pista1OK = false, pista2OK = false, victoryOK = false;
    int  pistActual = 1;
    bool enVictoria = false;
    bool muted      = false;

    // Efectos
    sf::SoundBuffer bufOpcion, bufDeath, bufCaja;
    sf::Sound       sndOpcion, sndDeath, sndCaja;
    bool bufOpcionOK = false, bufDeathOK = false, bufCajaOK = false;
};
