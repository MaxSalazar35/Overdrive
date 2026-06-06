// ============================================================
// Menu.cpp — Overdrive
// Menú principal cyberpunk con animación de fondo en parallax,
// selección de personaje por jugador y paneles de lore.
// ============================================================
#include "Menu.hpp"
#include "Constantes.hpp"
#include <cmath>
#include <iostream>

// ── Paleta neón ───────────────────────────────────────────────
static const sf::Color COL_ACENTO    = sf::Color(0, 220, 255);     // cian
static const sf::Color COL_ACENTO2   = sf::Color(255, 40, 160);    // magenta
static const sf::Color COL_BG        = sf::Color(6, 8, 20, 240);
static const sf::Color COL_PANEL     = sf::Color(12, 14, 32, 220);
static const sf::Color COL_TEXTO     = sf::Color(210, 220, 255);
static const sf::Color COL_DIM       = sf::Color(80, 90, 130);

// ── Colores por jugador ───────────────────────────────────────
static sf::Color colorJ(int idx) {
    if (idx == 0) return sf::Color(255, 60, 80);    // rojo J1
    return sf::Color(60, 180, 255);                   // azul J2
}

// ── Nombres de las opciones del menú ─────────────────────────
static const char* OPCIONES[4] = {
    "JUGAR",
    "COMO JUGAR",
    "AJUSTES",
    "SALIR"
};

// ═══════════════════════════════════════════════════════════════
// Constructor
// ═══════════════════════════════════════════════════════════════
Menu::Menu(sf::RenderWindow& v, sf::Music& m)
    : ventana(v), musica(m)
{
    cargarRecursos();
    registrarPersonajes();
}

// ── Cargar fuentes y fondos ───────────────────────────────────
void Menu::cargarRecursos()
{
    // Fuentes
    if (!fuente.loadFromFile("assets/fonts/Rajdhani-Bold.ttf"))
        fuente.loadFromFile("assets/fonts/Ring.ttf");
    fuenteTitulo = fuente;
    fuenteCargada = true;

    // Fondo parallax (mismas capas del juego)
    fondoCargado = texFondo1.loadFromFile("assets/images/bg_layer1.png")
                && texFondo2.loadFromFile("assets/images/bg_layer2.png")
                && texFondo3.loadFromFile("assets/images/bg_layer3.png");

    if (fondoCargado) {
        texFondo1.setRepeated(true);
        texFondo2.setRepeated(true);
        texFondo3.setRepeated(true);

        auto cfg = [&](sf::Sprite& spr, sf::Texture& tex) {
            spr.setTexture(tex);
            float scY = (float)ALTO_VENTANA / tex.getSize().y;
            spr.setScale(scY, scY);
        };
        cfg(sprFondo1, texFondo1);
        cfg(sprFondo2, texFondo2);
        cfg(sprFondo3, texFondo3);
    }
}

// ── Registrar personajes (se agregan más aquí) ────────────────
void Menu::registrarPersonajes()
{
    // JUGADORES HUMANOS
    personajes.push_back({
        "Axel \"VANDAL\" Reyes",
        "VANDAL",
        "assets/images/jugador1.png",    // spritesheet original
        "assets/images/jugador1.png",
        "El Rockero Temerario.\nCorre por la gloria y el ruido.\nSu armadura roja lleva cada choque como una medalla.",
        sf::Color(220, 40, 60)
    });
    personajes.push_back({
        "Jin \"COBALT\" Tanaka",
        "COBALT",
        "assets/images/jugador2.png",
        "assets/images/jugador2.png",
        "El Piloto de Precision.\nCada salto calculado al milimetro.\nSu traje azul jamas muestra una marca.",
        sf::Color(60, 140, 255)
    });
    personajes.push_back({
        "Kira \"APEX\" Vance",
        "APEX",
        "assets/images/jugador1.png",   // placeholder — reemplazar con sprite real
        "assets/images/jugador1.png",
        "La Buscavidas Callejera.\nCreció en los suburbios industriales.\nDeja una estela de neon verde al deslizarse.",
        sf::Color(60, 220, 80)
    });
    personajes.push_back({
        "Marcus \"VOLTAGE\" Briggs",
        "VOLTAGE",
        "assets/images/jugador2.png",   // placeholder
        "assets/images/jugador2.png",
        "El Mecánico del Bloque.\nModificó su armadura con chatarra corporativa.\nGrita de emoción en la velocidad máxima.",
        sf::Color(255, 200, 30)
    });
    personajes.push_back({
        "\"FADE\"",
        "FADE",
        "assets/images/jugador1.png",   // placeholder
        "assets/images/jugador1.png",
        "El Espectro Nocturno.\nNadie sabe su nombre real.\nGana y desaparece antes de que lleguen las patrullas.",
        sf::Color(140, 60, 220)
    });

    // Cargar texturas de perfil
    texPerfiles.resize(personajes.size());
    sprPerfiles.resize(personajes.size());
    for (int i = 0; i < (int)personajes.size(); ++i) {
        if (texPerfiles[i].loadFromFile(personajes[i].archivoPerfil)) {
            sprPerfiles[i].setTexture(texPerfiles[i]);
            // Recortar solo el primer frame idle (316x212 del spritesheet)
            if (texPerfiles[i].getSize().x >= 316)
                sprPerfiles[i].setTextureRect(sf::IntRect(0, 0, 316, 212));
        }
    }
}

// ═══════════════════════════════════════════════════════════════
// Bucle principal del menú
// ═══════════════════════════════════════════════════════════════
ResultadoMenu Menu::ejecutar()
{
    ResultadoMenu res;
    bool listo = false;

    while (!listo && ventana.isOpen())
    {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        tiempoTotal += dt;
        scrollFondo += dt * 40.f;   // parallax lento

        // ── Eventos ──────────────────────────────────────────
        sf::Event ev;
        while (ventana.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                res.salir = true;
                return res;
            }
        }

        // ── Input por pantalla ────────────────────────────────
        switch (pantalla) {
            case PantallaMenu::Principal:
                manejarInputPrincipal(listo, res);
                break;
            case PantallaMenu::ComoJugar:
                manejarInputComoJugar();
                break;
            case PantallaMenu::Ajustes:
                manejarInputAjustes();
                break;
            case PantallaMenu::SeleccionPersonaje:
                manejarInputSeleccion(listo, res);
                break;
            default: break;
        }

        // ── Dibujar ───────────────────────────────────────────
        ventana.clear(sf::Color(6, 8, 20));
        dibujarFondo();

        switch (pantalla) {
            case PantallaMenu::Principal:
                dibujarPrincipal();
                break;
            case PantallaMenu::ComoJugar:
                dibujarComoJugar();
                break;
            case PantallaMenu::Ajustes:
                dibujarAjustes();
                break;
            case PantallaMenu::SeleccionPersonaje:
                dibujarSeleccion();
                break;
            default: break;
        }

        ventana.display();
    }
    return res;
}

// ═══════════════════════════════════════════════════════════════
// INPUT
// ═══════════════════════════════════════════════════════════════

void Menu::manejarInputPrincipal(bool& listo, ResultadoMenu& res)
{
    bool arriba  = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool abajo   = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    if (arriba && !arribaPress) {
        opcionActual = (opcionActual - 1 + NUM_OPCIONES) % NUM_OPCIONES;
    }
    if (abajo && !abajoPress) {
        opcionActual = (opcionActual + 1) % NUM_OPCIONES;
    }
    if (confirma && !enterPress) {
        switch (opcionActual) {
            case 0: // JUGAR → selección de personajes
                pantalla = PantallaMenu::SeleccionPersonaje;
                jugadorActivo = 0;
                seleccionJ1 = 0;
                seleccionJ2 = 1;
                break;
            case 1: // CÓMO JUGAR
                pantalla = PantallaMenu::ComoJugar;
                break;
            case 2: // AJUSTES
                pantalla = PantallaMenu::Ajustes;
                break;
            case 3: // SALIR
                res.salir = true;
                listo = true;
                break;
        }
    }

    arribaPress = arriba;
    abajoPress  = abajo;
    enterPress  = confirma;
}

void Menu::manejarInputComoJugar()
{
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if ((esc && !escPress) || (confirma && !enterPress))
        pantalla = PantallaMenu::Principal;
    escPress   = esc;
    enterPress = confirma;
}

void Menu::manejarInputAjustes()
{
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    bool arriba  = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool abajo   = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::S);

    // Navegar las 2 opciones con arriba/abajo y toggle con Enter
    static int opAjuste = 0; // 0=música, 1=fullscreen
    if (arriba  && !arribaPress) opAjuste = 0;
    if (abajo   && !abajoPress)  opAjuste = 1;
    if (confirma && !enterPress) {
        if (opAjuste == 0) {
            musicaMuteada = !musicaMuteada;
            musica.setVolume(musicaMuteada ? 0.f : 50.f);
        } else {
            pantallaCompleta = !pantallaCompleta;
            if (pantallaCompleta)
                ventana.create(sf::VideoMode::getDesktopMode(),
                               "Overdrive", sf::Style::Fullscreen);
            else
                ventana.create(sf::VideoMode(ANCHO_VENTANA, ALTO_VENTANA),
                               "Overdrive", sf::Style::Close | sf::Style::Titlebar);
        }
    }
    if (esc && !escPress)
        pantalla = PantallaMenu::Principal;

    arribaPress = arriba;
    abajoPress  = abajo;
    enterPress  = confirma;
    escPress    = esc;
}

void Menu::manejarInputSeleccion(bool& listo, ResultadoMenu& res)
{
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool izq     = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool der     = sf::Keyboard::isKeyPressed(sf::Keyboard::Right)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    // J1 usa ← → Enter, J2 usa A D F (ya se usan por Box2D en juego)
    // Aquí para el menú igualmente: cualquiera navega con ← →, pero
    // J1 confirma con Enter y J2 confirma con F.
    bool confirmJ2 = sf::Keyboard::isKeyPressed(sf::Keyboard::F);

    int n = (int)personajes.size();

    if (jugadorActivo == 0) {
        // J1 elige
        if (izq  && !izqPress)  seleccionJ1 = (seleccionJ1 - 1 + n) % n;
        if (der  && !derPress)  seleccionJ1 = (seleccionJ1 + 1) % n;
        if (confirma && !enterPress) {
            jugadorActivo = 1;   // Ahora J2 elige
        }
    } else if (jugadorActivo == 1) {
        // J2 elige
        if (izq  && !izqPress)  seleccionJ2 = (seleccionJ2 - 1 + n) % n;
        if (der  && !derPress)  seleccionJ2 = (seleccionJ2 + 1) % n;
        if ((confirmJ2 || confirma) && !enterPress) {
            // Ambos confirman → ir al juego
            res.personaje1 = seleccionJ1;
            res.personaje2 = seleccionJ2;
            listo = true;
        }
    }

    if (esc && !escPress) {
        if (jugadorActivo == 1)
            jugadorActivo = 0;  // J2 cancela → vuelve a J1
        else
            pantalla = PantallaMenu::Principal;
    }

    izqPress   = izq;
    derPress   = der;
    enterPress = confirma || confirmJ2;
    escPress   = esc;
}

// ═══════════════════════════════════════════════════════════════
// DIBUJO
// ═══════════════════════════════════════════════════════════════

void Menu::dibujarFondo()
{
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    if (fondoCargado) {
        // Capa 1 — estática (cielo/luna)
        sprFondo1.setTextureRect(sf::IntRect(
            (int)(scrollFondo * 0.1f) % texFondo1.getSize().x, 0,
            (int)(W / sprFondo1.getScale().x) + 1, texFondo1.getSize().y));
        sprFondo1.setPosition(0, 0);
        ventana.draw(sprFondo1);

        // Capa 2 — parallax medio (edificios)
        sprFondo2.setTextureRect(sf::IntRect(
            (int)(scrollFondo * 0.4f) % texFondo2.getSize().x, 0,
            (int)(W / sprFondo2.getScale().x) + 1, texFondo2.getSize().y));
        sprFondo2.setPosition(0, 0);
        ventana.draw(sprFondo2);

        // Capa 3 — parallax frontal (tuberías)
        sprFondo3.setTextureRect(sf::IntRect(
            (int)(scrollFondo * 0.8f) % texFondo3.getSize().x, 0,
            (int)(W / sprFondo3.getScale().x) + 1, texFondo3.getSize().y));
        sprFondo3.setPosition(0, 0);
        ventana.draw(sprFondo3);
    }

    // Overlay oscuro para legibilidad
    sf::RectangleShape overlay({W, H});
    overlay.setFillColor(sf::Color(6, 8, 20, 160));
    ventana.draw(overlay);
}

// ── Pantalla Principal ─────────────────────────────────────────
void Menu::dibujarPrincipal()
{
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    // ── Líneas decorativas neón ───────────────────────────────
    float pulso = 0.5f + 0.5f * std::sin(tiempoTotal * 2.5f);
    sf::Color neon1(0, (sf::Uint8)(180 + 75*pulso), 255, 200);
    sf::Color neon2((sf::Uint8)(200 + 55*pulso), 40, 160, 200);

    dibujarLineaNeón(0, H * 0.12f, W, H * 0.12f, neon1, 1.5f);
    dibujarLineaNeón(0, H * 0.88f, W, H * 0.88f, neon1, 1.5f);
    dibujarLineaNeón(W * 0.5f - 2, H * 0.12f, W * 0.5f - 2, H * 0.88f, neon2, 1.f);

    // ── Logo OVERDRIVE ────────────────────────────────────────
    // Sombra
    sf::Text sombra = crearTexto("OVERDRIVE", 96, sf::Color(0,0,0,180),
                                  W * 0.5f + 4, H * 0.22f + 4, true);
    ventana.draw(sombra);

    // Texto principal con efecto de glitch sutil
    float glitch = std::sin(tiempoTotal * 7.3f) > 0.95f ? 3.f : 0.f;
    sf::Text titulo = crearTexto("OVERDRIVE", 96,
        sf::Color(255, (sf::Uint8)(255*pulso), 255), W * 0.5f + glitch, H * 0.22f, true);
    ventana.draw(titulo);

    // Subtítulo
    sf::Text sub = crearTexto("RACE OR DIE", 22, COL_ACENTO,
                               W * 0.5f, H * 0.33f, true);
    sub.setLetterSpacing(4.f);
    ventana.draw(sub);

    // ── Opciones ──────────────────────────────────────────────
    float startY = H * 0.45f;
    float step   = 72.f;
    float btnW   = 320.f;
    float btnH   = 52.f;

    for (int i = 0; i < NUM_OPCIONES; ++i) {
        bool sel = (i == opcionActual);
        float y  = startY + i * step;
        dibujarBotón(OPCIONES[i], W * 0.5f - btnW * 0.5f, y, btnW, btnH, sel, COL_ACENTO);
    }

    // ── Pie ────────────────────────────────────────────────────
    sf::Text pie = crearTexto("USE  W/S  OR  ARROWS  +  ENTER", 14,
                               COL_DIM, W * 0.5f, H * 0.93f, true);
    pie.setLetterSpacing(2.f);
    ventana.draw(pie);
}

// ── Pantalla Cómo Jugar ────────────────────────────────────────
void Menu::dibujarComoJugar()
{
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    // Panel central
    float pw = 700.f, ph = 460.f;
    float px = (W - pw) * 0.5f, py = (H - ph) * 0.5f;
    dibujarPanelOscuro(px, py, pw, ph, COL_ACENTO);

    // Título
    sf::Text titulo = crearTexto("COMO JUGAR", 42, COL_ACENTO,
                                  W * 0.5f, py + 20.f, true);
    ventana.draw(titulo);

    dibujarLineaNeón(px + 20, py + 72, px + pw - 20, py + 72, COL_ACENTO, 1.5f);

    // Columna J1
    float col1x = px + 40.f;
    float col2x = px + pw * 0.5f + 20.f;
    float ry = py + 95.f;
    float ls = 36.f;

    sf::Text tj1 = crearTexto("JUGADOR 1", 22, colorJ(0), col1x, ry);
    ventana.draw(tj1);
    ry += ls + 8;

    struct Ctrl { const char* accion; const char* tecla; };
    Ctrl ctrls1[] = {
        {"Moverse",  "FLECHA IZQ / DER"},
        {"Saltar",   "FLECHA ARRIBA"},
        {"Deslizar", "FLECHA ABAJO"},
        {"Gancho",   "SHIFT DERECHO"},
    };
    for (auto& c : ctrls1) {
        sf::Text ta = crearTexto(c.accion, 18, COL_TEXTO, col1x, ry);
        ventana.draw(ta);
        sf::Text tb = crearTexto(c.tecla,  16, COL_DIM,   col1x + 110, ry);
        ventana.draw(tb);
        ry += ls;
    }

    ry = py + 95.f;
    sf::Text tj2 = crearTexto("JUGADOR 2", 22, colorJ(1), col2x, ry);
    ventana.draw(tj2);
    ry += ls + 8;

    Ctrl ctrls2[] = {
        {"Moverse",  "A  /  D"},
        {"Saltar",   "W"},
        {"Deslizar", "S"},
        {"Gancho",   "SHIFT IZQUIERDO"},
    };
    for (auto& c : ctrls2) {
        sf::Text ta = crearTexto(c.accion, 18, COL_TEXTO, col2x, ry);
        ventana.draw(ta);
        sf::Text tb = crearTexto(c.tecla,  16, COL_DIM,   col2x + 110, ry);
        ventana.draw(tb);
        ry += ls;
    }

    // Regla del juego
    float ry2 = py + ph - 130.f;
    dibujarLineaNeón(px + 20, ry2 - 10, px + pw - 20, ry2 - 10, COL_ACENTO2, 1.f);

    sf::Text regla1 = crearTexto("MANTEN LA DELANTERA Y SOBREVIVE EN LAS PLATAFORMAS",
                                  18, sf::Color(255, 200, 80), W * 0.5f, ry2 + 5, true);
    ventana.draw(regla1);
    sf::Text regla2 = crearTexto("El jugador que quede sin vidas pierde.  3 vidas por jugador.",
                                  15, COL_TEXTO, W * 0.5f, ry2 + 32, true);
    ventana.draw(regla2);
    sf::Text regla3 = crearTexto("Los picos matan al instante.  Las cajas te frenan.",
                                  15, COL_TEXTO, W * 0.5f, ry2 + 55, true);
    ventana.draw(regla3);

    // Botón volver
    sf::Text volver = crearTexto("[ ESC  o  ENTER  para volver ]", 16, COL_DIM,
                                  W * 0.5f, py + ph - 28.f, true);
    ventana.draw(volver);
}

// ── Pantalla Ajustes ───────────────────────────────────────────
void Menu::dibujarAjustes()
{
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    float pw = 440.f, ph = 260.f;
    float px = (W - pw) * 0.5f, py = (H - ph) * 0.5f;
    dibujarPanelOscuro(px, py, pw, ph, COL_ACENTO2);

    sf::Text titulo = crearTexto("AJUSTES", 38, COL_ACENTO2, W * 0.5f, py + 18.f, true);
    ventana.draw(titulo);
    dibujarLineaNeón(px + 20, py + 62, px + pw - 20, py + 62, COL_ACENTO2, 1.5f);

    // Opción música
    static int opAjuste = 0;
    bool mSel = (opAjuste == 0);
    bool fSel = (opAjuste == 1);

    float oy = py + 85.f;
    // Fondo de opción seleccionada
    if (mSel) {
        sf::RectangleShape hl({pw - 40.f, 40.f});
        hl.setPosition(px + 20.f, oy - 4);
        hl.setFillColor(sf::Color(0, 180, 255, 30));
        hl.setOutlineColor(COL_ACENTO);
        hl.setOutlineThickness(1.f);
        ventana.draw(hl);
    }
    sf::Text lMusic = crearTexto("MUSICA", 22, mSel ? COL_ACENTO : COL_TEXTO,
                                   px + 40.f, oy);
    ventana.draw(lMusic);
    // Checkbox
    sf::RectangleShape cbox({24.f, 24.f});
    cbox.setPosition(px + pw - 70.f, oy - 2);
    cbox.setFillColor(sf::Color::Transparent);
    cbox.setOutlineColor(mSel ? COL_ACENTO : COL_DIM);
    cbox.setOutlineThickness(2.f);
    ventana.draw(cbox);
    if (!musicaMuteada) {
        sf::RectangleShape check({14.f, 14.f});
        check.setPosition(px + pw - 65.f, oy + 3);
        check.setFillColor(COL_ACENTO);
        ventana.draw(check);
    }
    sf::Text stMusic = crearTexto(musicaMuteada ? "MUTED" : "ON", 16,
        musicaMuteada ? COL_DIM : sf::Color(80,255,140), px + pw - 38.f, oy + 4);
    ventana.draw(stMusic);

    oy += 55.f;
    if (fSel) {
        sf::RectangleShape hl({pw - 40.f, 40.f});
        hl.setPosition(px + 20.f, oy - 4);
        hl.setFillColor(sf::Color(200, 40, 160, 30));
        hl.setOutlineColor(COL_ACENTO2);
        hl.setOutlineThickness(1.f);
        ventana.draw(hl);
    }
    sf::Text lFS = crearTexto("PANTALLA COMPLETA", 22, fSel ? COL_ACENTO2 : COL_TEXTO,
                                px + 40.f, oy);
    ventana.draw(lFS);

    sf::RectangleShape cbox2({24.f, 24.f});
    cbox2.setPosition(px + pw - 70.f, oy - 2);
    cbox2.setFillColor(sf::Color::Transparent);
    cbox2.setOutlineColor(fSel ? COL_ACENTO2 : COL_DIM);
    cbox2.setOutlineThickness(2.f);
    ventana.draw(cbox2);
    if (pantallaCompleta) {
        sf::RectangleShape check2({14.f, 14.f});
        check2.setPosition(px + pw - 65.f, oy + 3);
        check2.setFillColor(COL_ACENTO2);
        ventana.draw(check2);
    }
    sf::Text stFS = crearTexto(pantallaCompleta ? "ON" : "OFF", 16,
        pantallaCompleta ? sf::Color(80,255,140) : COL_DIM, px + pw - 38.f, oy + 4);
    ventana.draw(stFS);

    // Nota
    sf::Text nota = crearTexto("W/S para navegar  |  ENTER para cambiar  |  ESC para volver",
                                 14, COL_DIM, W * 0.5f, py + ph - 28.f, true);
    ventana.draw(nota);
}

// ── Pantalla Selección de Personaje ───────────────────────────
void Menu::dibujarSeleccion()
{
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    // Encabezado
    sf::Text titulo;
    std::string tituloStr = (jugadorActivo == 0)
        ? "JUGADOR 1  >  ELIGE TU PERSONAJE"
        : "JUGADOR 2  >  ELIGE TU PERSONAJE";
    sf::Color colActivo = colorJ(jugadorActivo);

    titulo = crearTexto(tituloStr, 28, colActivo, W * 0.5f, 30.f, true);
    ventana.draw(titulo);
    dibujarLineaNeón(0, 68, W, 68, colActivo, 2.f);

    // Fila de tarjetas de personajes
    int n = (int)personajes.size();
    float cardW = 180.f, cardH = 200.f;
    float totalW = n * cardW + (n - 1) * 20.f;
    float startX = (W - totalW) * 0.5f;
    float cardY  = 90.f;

    int selActual = (jugadorActivo == 0) ? seleccionJ1 : seleccionJ2;
    int selOtro   = (jugadorActivo == 0) ? seleccionJ2 : seleccionJ1;

    for (int i = 0; i < n; ++i) {
        float cx = startX + i * (cardW + 20.f);
        bool  esSel   = (i == selActual);
        bool  esOtro  = (i == selOtro && jugadorActivo == 1);
        sf::Color bordeColor = esSel  ? colActivo
                             : esOtro ? colorJ(1 - jugadorActivo)
                             : COL_DIM;
        float escala = esSel ? 1.08f : 1.f;
        float offY   = esSel ? -8.f : 0.f;

        // Panel de la tarjeta
        sf::RectangleShape card({cardW * escala, cardH * escala});
        card.setPosition(cx - (cardW * escala - cardW) * 0.5f, cardY + offY);
        card.setFillColor(esSel ? sf::Color(20, 24, 50, 230) : COL_PANEL);
        card.setOutlineColor(bordeColor);
        card.setOutlineThickness(esSel ? 2.5f : 1.f);
        ventana.draw(card);

        // Sprite del personaje (primer frame)
        if (i < (int)sprPerfiles.size() && texPerfiles[i].getSize().x > 0) {
            sf::Sprite& spr = sprPerfiles[i];
            float fw = 316.f, fh = 212.f;
            float sc = std::min(cardW / fw, (cardH - 40.f) / fh) * escala * 0.88f;
            spr.setScale(sc, sc);
            float sx = cx + (cardW * escala - fw * sc) * 0.5f
                          - (cardW * escala - cardW) * 0.5f;
            float sy = cardY + offY + 8.f;
            spr.setPosition(sx, sy);
            // Tint con el color del personaje si está seleccionado
            if (esSel) spr.setColor(sf::Color(255, 255, 255, 255));
            else        spr.setColor(sf::Color(130, 130, 150, 200));
            ventana.draw(spr);
        }

        // Alias debajo
        sf::Text alias = crearTexto(personajes[i].alias, 15,
            esSel ? colActivo : COL_DIM,
            cx + cardW * 0.5f, cardY + cardH - 22.f + offY, true);
        ventana.draw(alias);

        // Indicador de quién lo tiene seleccionado
        if (i == seleccionJ1 && jugadorActivo == 1) {
            sf::Text mark = crearTexto("J1", 12, colorJ(0),
                cx + cardW - 22.f, cardY + 4.f + offY);
            ventana.draw(mark);
        }
        if (i == seleccionJ2 && jugadorActivo == 0) {
            sf::Text mark = crearTexto("J2", 12, colorJ(1),
                cx + cardW - 22.f, cardY + 4.f + offY);
            ventana.draw(mark);
        }
    }

    // ── Panel de lore del personaje seleccionado ─────────────
    dibujarLineaNeón(0, cardY + cardH + 18, W, cardY + cardH + 18, colActivo, 1.5f);

    float loreY = cardY + cardH + 30.f;
    const DatosPersonaje& dp = personajes[selActual];

    // Nombre completo
    sf::Text nombre = crearTexto(dp.nombre, 28, colActivo, W * 0.5f, loreY, true);
    ventana.draw(nombre);

    // Lore — partir por \n
    float ly = loreY + 44.f;
    std::string lore = dp.lore;
    size_t pos = 0;
    while ((pos = lore.find('\n')) != std::string::npos) {
        std::string linea = lore.substr(0, pos);
        sf::Text tl = crearTexto(linea, 17, COL_TEXTO, W * 0.5f, ly, true);
        ventana.draw(tl);
        lore = lore.substr(pos + 1);
        ly += 26.f;
    }
    sf::Text tl = crearTexto(lore, 17, COL_TEXTO, W * 0.5f, ly, true);
    ventana.draw(tl);

    // ── Instrucciones ─────────────────────────────────────────
    float instrY = H - 55.f;
    dibujarLineaNeón(0, instrY - 12, W, instrY - 12, COL_DIM, 1.f);

    std::string instrStr;
    if (jugadorActivo == 0)
        instrStr = "J1:  FLECHAS IZQ/DER  para cambiar  |  ENTER para confirmar  |  ESC para volver";
    else
        instrStr = "J2:  FLECHAS IZQ/DER  para cambiar  |  F o ENTER para confirmar  |  ESC para cancelar";

    sf::Text instr = crearTexto(instrStr, 15, COL_DIM, W * 0.5f, instrY, true);
    ventana.draw(instr);
}

// ═══════════════════════════════════════════════════════════════
// UTILIDADES VISUALES
// ═══════════════════════════════════════════════════════════════

sf::Text Menu::crearTexto(const std::string& str, unsigned size,
                            sf::Color color, float x, float y, bool centrarX)
{
    sf::Text t;
    t.setFont(fuente);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setString(str);
    if (centrarX) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width * 0.5f, b.top);
    }
    t.setPosition(x, y);
    return t;
}

void Menu::dibujarPanelOscuro(float x, float y, float w, float h, sf::Color borde)
{
    sf::RectangleShape panel({w, h});
    panel.setPosition(x, y);
    panel.setFillColor(COL_PANEL);
    panel.setOutlineColor(borde);
    panel.setOutlineThickness(2.f);
    ventana.draw(panel);
}

void Menu::dibujarLineaNeón(float x1, float y1, float x2, float y2,
                              sf::Color color, float grosor)
{
    float dx = x2 - x1, dy = y2 - y1;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 1.f) return;

    sf::RectangleShape linea({len, grosor});
    linea.setPosition(x1, y1);
    linea.setFillColor(color);
    linea.setRotation(std::atan2(dy, dx) * 180.f / 3.14159f);
    ventana.draw(linea);
}

void Menu::dibujarBotón(const std::string& txt, float x, float y,
                         float w, float h, bool sel, sf::Color color)
{
    // Fondo
    sf::RectangleShape btn({w, h});
    btn.setPosition(x, y);
    if (sel) {
        btn.setFillColor(sf::Color(color.r/6, color.g/6, color.b/6, 200));
        btn.setOutlineColor(color);
        btn.setOutlineThickness(2.f);
    } else {
        btn.setFillColor(sf::Color(20, 22, 40, 180));
        btn.setOutlineColor(COL_DIM);
        btn.setOutlineThickness(1.f);
    }
    ventana.draw(btn);

    // Acento lateral izquierdo si seleccionado
    if (sel) {
        sf::RectangleShape acento({4.f, h});
        acento.setPosition(x, y);
        acento.setFillColor(color);
        ventana.draw(acento);
    }

    // Texto centrado en el botón
    sf::Text t;
    t.setFont(fuente);
    t.setCharacterSize(22);
    t.setFillColor(sel ? color : COL_TEXTO);
    t.setString(txt);
    t.setLetterSpacing(2.f);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
    t.setPosition(x + w * 0.5f, y + h * 0.5f);
    ventana.draw(t);
}
