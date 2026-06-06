#pragma once
// ============================================================
// Menu.hpp — Overdrive
// Menú principal con: Play, Cómo Jugar, Ajustes, Salir
// Selección de personaje (4 por jugador, con historial y lore)
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <functional>

// ── Datos de cada personaje ──────────────────────────────────
struct DatosPersonaje {
    std::string nombre;       // Nombre para mostrar en pantalla
    std::string alias;        // Alias estilo "VANDAL", "COBALT"…
    std::string archivoSprite;// Ruta al spritesheet  (assets/images/…)
    std::string archivoPerfil;// Imagen de perfil para la selección
    std::string lore;         // Historia corta (2-3 líneas)
    sf::Color   color;        // Color representativo del personaje
};

// ── Pantallas del menú ────────────────────────────────────────
enum class PantallaMenu {
    Principal,
    ComoJugar,
    Ajustes,
    SeleccionPersonaje,
    Salir
};

// ── Resultado que devuelve el menú al main ────────────────────
struct ResultadoMenu {
    bool salir = false;          // El usuario eligió Exit
    int  personaje1 = 0;         // Índice del personaje del J1
    int  personaje2 = 1;         // Índice del personaje del J2
};

// ============================================================
// Clase principal del menú
// ============================================================
class Menu {
public:
    Menu(sf::RenderWindow& ventana, sf::Music& musica);

    // Corre el bucle del menú. Devuelve datos cuando el jugador
    // pulsa Play y termina la selección de personajes.
    ResultadoMenu ejecutar();

private:
    // ── Referencias ─────────────────────────────────────────
    sf::RenderWindow& ventana;
    sf::Music&        musica;

    // ── Recursos ─────────────────────────────────────────────
    sf::Font  fuente;
    sf::Font  fuenteTitulo;
    bool      fuenteCargada = false;

    // Fondo animado (las mismas capas del juego)
    sf::Texture texFondo1, texFondo2, texFondo3;
    sf::Sprite  sprFondo1, sprFondo2, sprFondo3;
    bool        fondoCargado = false;
    float       scrollFondo  = 0.f;

    // Personajes disponibles
    std::vector<DatosPersonaje>   personajes;
    std::vector<sf::Texture>      texPerfiles;   // imágenes para la selección
    std::vector<sf::Sprite>       sprPerfiles;

    // ── Estado de la pantalla ─────────────────────────────────
    PantallaMenu pantalla = PantallaMenu::Principal;

    // Menú principal — índice de opción seleccionada
    int opcionActual = 0;
    static constexpr int NUM_OPCIONES = 4;

    // Ajustes
    bool musicaMuteada   = false;
    bool pantallaCompleta = false;

    // Selección de personajes
    int seleccionJ1 = 0;
    int seleccionJ2 = 1;
    int jugadorActivo = 0; // 0 = J1 eligiendo, 1 = J2 eligiendo, 2 = ambos listos

    // Animación / tiempo
    float tiempoTotal = 0.f;
    sf::Clock reloj;

    // Input — evitar repetición
    bool enterPress   = false;
    bool escPress     = false;
    bool arribaPress  = false;
    bool abajoPress   = false;
    bool izqPress     = false;
    bool derPress     = false;

    // ── Métodos privados ──────────────────────────────────────
    void cargarRecursos();
    void registrarPersonajes();

    // Dibujado por pantalla
    void dibujarFondo();
    void dibujarPrincipal();
    void dibujarComoJugar();
    void dibujarAjustes();
    void dibujarSeleccion();

    // Input por pantalla
    void manejarInputPrincipal(bool& listo, ResultadoMenu& res);
    void manejarInputComoJugar();
    void manejarInputAjustes();
    void manejarInputSeleccion(bool& listo, ResultadoMenu& res);

    // Utilidades visuales
    sf::Text crearTexto(const std::string& str, unsigned size,
                        sf::Color color, float x, float y,
                        bool centrarX = false);
    void dibujarPanelOscuro(float x, float y, float w, float h,
                             sf::Color borde = sf::Color(80,80,120,200));
    void dibujarLineaNeón(float x1, float y1, float x2, float y2,
                          sf::Color color, float grosor = 2.f);
    void dibujarBotón(const std::string& txt, float x, float y,
                      float w, float h, bool seleccionado, sf::Color color);
};
