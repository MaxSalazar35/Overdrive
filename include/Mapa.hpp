#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <vector>
#include <string>

// ============================================================
// Mapa.hpp — Overdrive (Box2D v3)
// ============================================================

enum class TipoPlat {
    Terreno,
    Rampa,
    Pico,
};

struct Plataforma {
    sf::RectangleShape rectVisual;
    sf::ConvexShape    convVisual;
    bool               usaConvex = false;

    b2BodyId cuerpo;
    bool     valido         = false;
    int      chunk          = 0;
    bool     esGancheable   = true;
    bool     esPico         = false;
    bool     esTechoGancho  = false;
    TipoPlat tipo           = TipoPlat::Terreno;
};

struct CajaItem {
    sf::RectangleShape visual;
    sf::Vector2f       posBase;
    bool               activa       = true;
    float              timerRespawn = 0.f;
    sf::Clock          reloj;
    int                chunk        = 0;
};

class Mapa {
public:
    explicit Mapa(b2WorldId worldId);
    ~Mapa();

    void update(float dt, float camaraX);
    void drawFondo(sf::RenderWindow& v, float camaraX);
    void drawPlataformas(sf::RenderWindow& v);
    void drawItems(sf::RenderWindow& v);
    void resetear();

    float getAncho()      const { return anchoPx; }
    float getAlto()       const { return altoPx;  }
    float getAnchoTotal() const;

    std::vector<CajaItem>&   getCajas()       { return cajas; }
    std::vector<Plataforma>& getPlataformas() { return plataformas; }
    sf::Vector2f spawnJugador(int id, float camaraX = 0.f) const;

private:
    b2WorldId worldId;
    std::vector<Plataforma> plataformas;
    std::vector<CajaItem>   cajas;

    sf::Texture texCapas[3];
    sf::Sprite  sprCapas[3];
    bool        capaCargada[3] = {false,false,false};
    float       velParallax[3] = {0.1f, 0.25f, 0.5f};
    bool        modoFallback   = false;

    static constexpr float anchoPx = 3840.f;
    static constexpr float altoPx  = 720.f;

    int chunkMin      = 0;
    int chunkMax      = 1;
    int contadorChunk = 0;

    void generarChunk(int idx);
    void eliminarChunk(int idx);

    struct DefTerreno {
        float x, y, w;
        bool  gancheable    = true;
        float h             = 0.f;     // 0 = usa PLAT_H
        bool  esTechoGancho = false;
        bool  noExtender    = false;   // true = NO extender hasta SUELO_Y (ej: chunk D con picos)
    };

    struct DefRampa {
        float x, ySup, w, hIzq, hDer;
    };

    struct DefPico {
        float x, y, w;
        int   cantidad;
    };

    struct DefCaja { float x, y; };

    static const std::vector<DefTerreno> TERRENO_A;
    static const std::vector<DefTerreno> TERRENO_B;
    static const std::vector<DefTerreno> TERRENO_C;
    static const std::vector<DefTerreno> TERRENO_D;

    static const std::vector<DefRampa> RAMPAS_A;
    static const std::vector<DefRampa> RAMPAS_B;
    static const std::vector<DefRampa> RAMPAS_C;
    static const std::vector<DefRampa> RAMPAS_D;

    static const std::vector<DefPico> PICOS_A;
    static const std::vector<DefPico> PICOS_B;
    static const std::vector<DefPico> PICOS_C;
    static const std::vector<DefPico> PICOS_D;

    static const std::vector<DefCaja> PLANTILLA_CAJA;

    void generarConPlantilla(int idx,
        const std::vector<DefTerreno>& terrenos,
        const std::vector<DefRampa>&   rampas,
        const std::vector<DefPico>&    picos,
        const std::vector<DefCaja>&    cajas_def);

    void agregarTerreno(float x, float y, float w, float h,
                        bool gancheable, int chunk,
                        bool esTecho = false, bool noExtender = false);

    void agregarRampa(float x, float y, float w, float hIzq, float hDer,
                      int chunk);

    void agregarPicos(float x, float y, float w, int cantidad, int chunk);

    void agregarCaja(float x, float y, int chunk);
    void destruirFisicas();
};
