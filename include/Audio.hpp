#pragma once
// ============================================================
// Audio.hpp — Overdrive
// Sistema de audio centralizado:
//  - Música alterna: musica.ogg ↔ musica2.ogg
//  - victorysong.ogg: solo al ganar el juego final
//  - Efectos: opcion (navegar), seleccionar (confirmar),
//             salir (esc/volver), death, cajas
// ============================================================
#include <SFML/Audio.hpp>

class SistemaAudio {
public:
    SistemaAudio() {
        // Música alterna
        pista1OK = musica1.openFromFile("assets/music/musica.ogg");
        pista2OK = musica2.openFromFile("assets/sounds/musica2.ogg");
        if (pista1OK) { musica1.setLoop(false); musica1.setVolume(50.f); }
        if (pista2OK) { musica2.setLoop(false); musica2.setVolume(50.f); }

        // Victoria
        victoryOK = victory.openFromFile("assets/sounds/victorysong.ogg");
        if (victoryOK) { victory.setLoop(false); victory.setVolume(90.f); }

        // Efectos
        auto load = [](sf::SoundBuffer& buf, sf::Sound& snd,
                       const char* path, float vol, bool& ok) {
            ok = buf.loadFromFile(path);
            if (ok) { snd.setBuffer(buf); snd.setVolume(vol); }
        };
        load(bufOpcion,     sndOpcion,     "assets/sounds/opcion.ogg",     85.f, bufOpcionOK);
        load(bufSeleccionar,sndSeleccionar,"assets/sounds/seleccionar.ogg", 90.f, bufSeleccionarOK);
        load(bufSalir,      sndSalir,      "assets/sounds/salir.ogg",       85.f, bufSalirOK);
        load(bufDeath,      sndDeath,      "assets/sounds/death.ogg",      100.f, bufDeathOK);
        load(bufCaja,       sndCaja,       "assets/sounds/cajas.ogg",       85.f, bufCajaOK);
    }

    // ── Música principal ──────────────────────────────────────
    void iniciarMusica() {
        victory.stop(); muted = false; enVictoria = false;
        if (pista1OK) { musica1.play(); pistActual = 1; }
        else if (pista2OK) { musica2.play(); pistActual = 2; }
    }

    void updateMusica() {
        if (muted || enVictoria) return;
        if (pistActual == 1 && musica1.getStatus() == sf::Music::Stopped) {
            if (pista2OK) { musica2.play(); pistActual = 2; }
            else if (pista1OK) musica1.play();
        } else if (pistActual == 2 && musica2.getStatus() == sf::Music::Stopped) {
            if (pista1OK) { musica1.play(); pistActual = 1; }
            else if (pista2OK) musica2.play();
        }
    }

    // ── Victoria ──────────────────────────────────────────────
    void iniciarVictoria() {
        musica1.pause(); musica2.pause();
        enVictoria = true;
        if (victoryOK && !muted) victory.play();
    }

    void detenerVictoria() {
        victory.stop(); enVictoria = false;
        if (!muted) {
            if (pistActual == 1 && pista1OK) musica1.play();
            else if (pista2OK) musica2.play();
        }
    }

    // ── Mute ─────────────────────────────────────────────────
    void setMuted(bool m) {
        muted = m;
        musica1.setVolume(m ? 0.f : 50.f);
        musica2.setVolume(m ? 0.f : 50.f);
        victory.setVolume(m ? 0.f : 70.f);
    }
    bool isMuted() const { return muted; }

    sf::Music& getMusicaRef() {
        return (pistActual == 2 && pista2OK) ? musica2 : musica1;
    }

    // ── Efectos ───────────────────────────────────────────────
    void playOpcion()      { if (bufOpcionOK      && !muted) { sndOpcion.stop();     sndOpcion.play();     } }
    void playSeleccionar() { if (bufSeleccionarOK && !muted) { sndSeleccionar.stop();sndSeleccionar.play();} }
    void playSalir()       { if (bufSalirOK       && !muted) { sndSalir.stop();      sndSalir.play();      } }
    void playDeath()       { if (bufDeathOK       && !muted) { sndDeath.stop();      sndDeath.play();      } }
    void playCaja()        { if (bufCajaOK        && !muted) { sndCaja.stop();       sndCaja.play();       } }

private:
    sf::Music musica1, musica2, victory;
    bool pista1OK=false, pista2OK=false, victoryOK=false;
    int  pistActual=1;
    bool enVictoria=false, muted=false;

    sf::SoundBuffer bufOpcion, bufSeleccionar, bufSalir, bufDeath, bufCaja;
    sf::Sound       sndOpcion, sndSeleccionar, sndSalir, sndDeath, sndCaja;
    bool bufOpcionOK=false, bufSeleccionarOK=false, bufSalirOK=false,
         bufDeathOK=false, bufCajaOK=false;
};
