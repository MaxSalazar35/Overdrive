#pragma once
// ============================================================
// Menu.hpp — Overdrive v3
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>

struct DatosPersonaje {
    std::string nombre;
    std::string alias;
    std::string archivoSprite;  // spritesheet para el juego
    std::string archivoPerfil;  // imagen victory para el menú
    std::string lore;
    sf::Color   color;
};

enum class PantallaMenu { Principal, ComoJugar, Ajustes, SeleccionPersonaje, Salir };

struct DatosPersonajeElegido {
    std::string sprite;
    std::string victory;
    std::string alias;
    sf::Color   color;
};

struct ResultadoMenu {
    bool salir      = false;
    int  personaje1 = 0;
    int  personaje2 = 1;
    DatosPersonajeElegido datosJ1;
    DatosPersonajeElegido datosJ2;
};

class Menu {
public:
    Menu(sf::RenderWindow& ventana, sf::Music& musica);
    ResultadoMenu ejecutar();

    // Devuelve el path del spritesheet del personaje elegido
    std::string getSpriteJ1() const {
        return (personaje1idx < (int)personajes.size())
            ? personajes[personaje1idx].archivoSprite : "assets/images/vandal.png";
    }
    std::string getSpriteJ2() const {
        return (personaje2idx < (int)personajes.size())
            ? personajes[personaje2idx].archivoSprite : "assets/images/cobalt.png";
    }

private:
    sf::RenderWindow& ventana;
    sf::Music&        musica;

    sf::Font fuente, fuenteTitulo;
    bool     fuenteCargada = false;

    // Sonido de opción
    sf::SoundBuffer bufOpcion;
    sf::Sound       sndOpcion;
    bool            sndOpcionOK = false;

    sf::Texture texFondo1, texFondo2, texFondo3;
    sf::Sprite  sprFondo1, sprFondo2, sprFondo3;
    bool        fondoCargado = false;
    float       scrollFondo  = 0.f;

    std::vector<DatosPersonaje> personajes;
    std::vector<sf::Texture>    texPerfiles;   // victory images
    std::vector<sf::Sprite>     sprPerfiles;
    std::vector<sf::Texture>    texSprites;    // spritesheets (para el juego)

    PantallaMenu pantalla    = PantallaMenu::Principal;
    int opcionActual = 0;
    static constexpr int NUM_OPCIONES = 4;

    // Ajustes — solo música (pantalla completa eliminada)
    bool musicaMuteada = false;
    int  opAjuste      = 0;

    // Selección de personajes
    int seleccionJ1   = 0;
    int seleccionJ2   = 1;
    int jugadorActivo = 0;
    int personaje1idx = 0;
    int personaje2idx = 1;

    float tiempoTotal = 0.f;
    sf::Clock reloj;

    bool enterPress  = false;
    bool escPress    = false;
    bool arribaPress = false;
    bool abajoPress  = false;
    bool izqPress    = false;
    bool derPress    = false;

    void cargarRecursos();
    void registrarPersonajes();

    void dibujarFondo();
    void dibujarPrincipal();
    void dibujarComoJugar();
    void dibujarAjustes();
    void dibujarSeleccion();

    void manejarInputPrincipal(bool& listo, ResultadoMenu& res);
    void manejarInputComoJugar();
    void manejarInputAjustes();
    void manejarInputSeleccion(bool& listo, ResultadoMenu& res);

    sf::Text crearTexto(const std::string& str, unsigned size,
                        sf::Color color, float x, float y, bool centrarX = false);
    void dibujarPanelOscuro(float x, float y, float w, float h,
                             sf::Color borde = sf::Color(80,80,120,200));
    void dibujarLineaNeon(float x1, float y1, float x2, float y2,
                          sf::Color color, float grosor = 2.f);
    void dibujarBoton(const std::string& txt, float x, float y,
                      float w, float h, bool sel, sf::Color color);
};
