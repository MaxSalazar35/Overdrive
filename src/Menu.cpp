// ============================================================
// Menu.cpp — Overdrive  (v3 — fixes completos)
// ============================================================
#include "Menu.hpp"
#include "Constantes.hpp"
#include <cmath>
#include <iostream>

static const sf::Color COL_ACENTO  = sf::Color(0, 220, 255);
static const sf::Color COL_ACENTO2 = sf::Color(255, 40, 160);
static const sf::Color COL_PANEL   = sf::Color(12, 14, 32, 220);
static const sf::Color COL_TEXTO   = sf::Color(210, 220, 255);
static const sf::Color COL_DIM     = sf::Color(80, 90, 130);

static sf::Color colorJ(int idx) {
    return (idx == 0) ? sf::Color(255, 60, 80) : sf::Color(60, 180, 255);
}

static const char* OPCIONES[4] = { "JUGAR", "COMO JUGAR", "AJUSTES", "SALIR" };

// ─────────────────────────────────────────────────────────────
Menu::Menu(sf::RenderWindow& v, sf::Music& m) : ventana(v), musica(m) {
    cargarRecursos();
    registrarPersonajes();
}

void Menu::cargarRecursos() {
    if (!fuente.loadFromFile("assets/fonts/Rajdhani-Bold.ttf"))
        fuente.loadFromFile("assets/fonts/Ring.ttf");
    fuenteTitulo  = fuente;
    fuenteCargada = true;

    fondoCargado =  texFondo1.loadFromFile("assets/images/bg_layer1.png")
                 && texFondo2.loadFromFile("assets/images/bg_layer2.png")
                 && texFondo3.loadFromFile("assets/images/bg_layer3.png");

    if (fondoCargado) {
        texFondo1.setRepeated(true);
        texFondo2.setRepeated(true);
        texFondo3.setRepeated(true);
        auto cfg = [&](sf::Sprite& spr, sf::Texture& tex) {
            spr.setTexture(tex);
            float sc = (float)ALTO_VENTANA / tex.getSize().y;
            spr.setScale(sc, sc);
        };
        cfg(sprFondo1, texFondo1);
        cfg(sprFondo2, texFondo2);
        cfg(sprFondo3, texFondo3);
    }
}

void Menu::registrarPersonajes() {
    // sprite, perfil (victory), lore sin caracteres problemáticos
    struct Info {
        const char* nombre; const char* alias;
        const char* sprite; const char* victory;
        const char* lore;
        sf::Color color;
    };

    Info infos[] = {
        {
            "Axel \"VANDAL\" Reyes", "VANDAL",
            "assets/images/vandal.png", "assets/images/vandal_victory.png",
            "El Rockero Temerario.\nCorre por la gloria y el ruido.\nSu armadura roja lleva cada choque como una medalla.",
            sf::Color(220, 40, 60)
        },
        {
            "Jin \"COBALT\" Tanaka", "COBALT",
            "assets/images/cobalt.png", "assets/images/cobalt_victory.png",
            "El Piloto de Precision.\nCada salto calculado al milimetro.\nSu traje azul jamas muestra una marca.",
            sf::Color(60, 140, 255)
        },
        {
            "Kira \"APEX\" Vance", "APEX",
            "assets/images/apex.png", "assets/images/apex_victory.png",
            "La Buscavidas Callejera.\nCrecio en los suburbios industriales.\nDeja una estela de neon verde al deslizarse.",
            sf::Color(60, 220, 80)
        },
        {
            "Marcus \"VOLTAGE\" Briggs", "VOLTAGE",
            "assets/images/voltage.png", "assets/images/voltage_victory.png",
            "El Mecanico del Bloque.\nModifico su armadura con chatarra corporativa.\nGrita de emocion en la velocidad maxima.",
            sf::Color(255, 200, 30)
        },
        {
            "\"FADE\"", "FADE",
            "assets/images/fade.png", "assets/images/fade_victory.png",
            "El Espectro Nocturno.\nNadie sabe su nombre real.\nGana y desaparece antes de que lleguen las patrullas.",
            sf::Color(140, 60, 220)
        },
    };

    for (auto& info : infos) {
        personajes.push_back({
            info.nombre, info.alias,
            info.sprite, info.victory,
            info.lore,   info.color
        });
    }

    // Cargar texturas: spritesheet para el juego, victory para el menú
    int n = (int)personajes.size();
    texPerfiles.resize(n);
    sprPerfiles.resize(n);
    texSprites.resize(n);

    for (int i = 0; i < n; ++i) {
        // Imagen de victoria como perfil en la selección
        texPerfiles[i].loadFromFile(personajes[i].archivoPerfil);
        sprPerfiles[i].setTexture(texPerfiles[i]);
        // NO recortamos: es una imagen completa (no spritesheet)

        // Spritesheet para el juego
        texSprites[i].loadFromFile(personajes[i].archivoSprite);
    }
}

// ─────────────────────────────────────────────────────────────
ResultadoMenu Menu::ejecutar() {
    ResultadoMenu res;
    bool listo = false;

    while (!listo && ventana.isOpen()) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        tiempoTotal += dt;
        scrollFondo += dt * 40.f;

        sf::Event ev;
        while (ventana.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) { res.salir = true; return res; }
        }

        switch (pantalla) {
            case PantallaMenu::Principal:          manejarInputPrincipal(listo, res); break;
            case PantallaMenu::ComoJugar:          manejarInputComoJugar();           break;
            case PantallaMenu::Ajustes:            manejarInputAjustes();             break;
            case PantallaMenu::SeleccionPersonaje: manejarInputSeleccion(listo, res); break;
            default: break;
        }

        ventana.clear(sf::Color(6, 8, 20));
        dibujarFondo();

        switch (pantalla) {
            case PantallaMenu::Principal:          dibujarPrincipal();  break;
            case PantallaMenu::ComoJugar:          dibujarComoJugar();  break;
            case PantallaMenu::Ajustes:            dibujarAjustes();    break;
            case PantallaMenu::SeleccionPersonaje: dibujarSeleccion();  break;
            default: break;
        }

        ventana.display();
    }
    return res;
}

// ─────────────────────────────────────────────────────────────
// INPUT
// ─────────────────────────────────────────────────────────────
void Menu::manejarInputPrincipal(bool& listo, ResultadoMenu& res) {
    bool arriba   = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
                 || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool abajo    = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
                 || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool confirma = sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                 || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    if (arriba && !arribaPress)
        opcionActual = (opcionActual - 1 + NUM_OPCIONES) % NUM_OPCIONES;
    if (abajo && !abajoPress)
        opcionActual = (opcionActual + 1) % NUM_OPCIONES;

    if (confirma && !enterPress) {
        switch (opcionActual) {
            case 0: pantalla = PantallaMenu::SeleccionPersonaje;
                    jugadorActivo = 0; seleccionJ1 = 0; seleccionJ2 = 1; break;
            case 1: pantalla = PantallaMenu::ComoJugar;  break;
            case 2: pantalla = PantallaMenu::Ajustes;    break;
            case 3: res.salir = true; listo = true;      break;
        }
    }
    arribaPress = arriba; abajoPress = abajo; enterPress = confirma;
}

void Menu::manejarInputComoJugar() {
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if ((esc && !escPress) || (confirma && !enterPress))
        pantalla = PantallaMenu::Principal;
    escPress = esc; enterPress = confirma;
}

void Menu::manejarInputAjustes() {
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    bool arriba  = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool abajo   = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::S);

    if (arriba  && !arribaPress) opAjuste = 0;
    if (abajo   && !abajoPress)  opAjuste = 1;

    if (confirma && !enterPress) {
        if (opAjuste == 0) {
            musicaMuteada = !musicaMuteada;
            musica.setVolume(musicaMuteada ? 0.f : 50.f);
        }
        // opAjuste==1 reservado para futuro — pantalla completa eliminada
    }
    if (esc && !escPress) pantalla = PantallaMenu::Principal;

    arribaPress = arriba; abajoPress = abajo; enterPress = confirma; escPress = esc;
}

void Menu::manejarInputSeleccion(bool& listo, ResultadoMenu& res) {
    bool esc     = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
    bool izq     = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool der     = sf::Keyboard::isKeyPressed(sf::Keyboard::Right)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool confirma= sf::Keyboard::isKeyPressed(sf::Keyboard::Return)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    bool confirmJ2 = sf::Keyboard::isKeyPressed(sf::Keyboard::F);

    int n = (int)personajes.size();

    if (jugadorActivo == 0) {
        if (izq && !izqPress) seleccionJ1 = (seleccionJ1 - 1 + n) % n;
        if (der && !derPress) seleccionJ1 = (seleccionJ1 + 1) % n;
        if (confirma && !enterPress) jugadorActivo = 1;
    } else {
        if (izq && !izqPress) seleccionJ2 = (seleccionJ2 - 1 + n) % n;
        if (der && !derPress) seleccionJ2 = (seleccionJ2 + 1) % n;
        if ((confirmJ2 || confirma) && !enterPress) {
            res.personaje1 = seleccionJ1;
            res.personaje2 = seleccionJ2;
            // Llenar datos completos de cada personaje elegido
            if (seleccionJ1 < (int)personajes.size()) {
                const auto& p = personajes[seleccionJ1];
                res.datosJ1 = { p.archivoSprite, p.archivoPerfil, p.alias, p.color };
            }
            if (seleccionJ2 < (int)personajes.size()) {
                const auto& p = personajes[seleccionJ2];
                res.datosJ2 = { p.archivoSprite, p.archivoPerfil, p.alias, p.color };
            }
            listo = true;
        }
    }

    if (esc && !escPress) {
        if (jugadorActivo == 1) jugadorActivo = 0;
        else                    pantalla = PantallaMenu::Principal;
    }

    izqPress = izq; derPress = der;
    enterPress = confirma || confirmJ2;
    escPress   = esc;
}

// ─────────────────────────────────────────────────────────────
// DIBUJO
// ─────────────────────────────────────────────────────────────
void Menu::dibujarFondo() {
    float W = (float)ANCHO_VENTANA;
    if (!fondoCargado) return;

    auto drawLayer = [&](sf::Sprite& spr, sf::Texture& tex, float speed) {
        float sc = spr.getScale().x;
        int texW = tex.getSize().x;
        int visW = (int)(W / sc) + 2;
        int offX = (int)(scrollFondo * speed) % texW;
        spr.setTextureRect(sf::IntRect(offX, 0, visW, tex.getSize().y));
        spr.setPosition(0, 0);
        ventana.draw(spr);
    };

    drawLayer(sprFondo1, texFondo1, 0.10f);
    drawLayer(sprFondo2, texFondo2, 0.40f);
    drawLayer(sprFondo3, texFondo3, 0.80f);

    sf::RectangleShape overlay({W, (float)ALTO_VENTANA});
    overlay.setFillColor(sf::Color(6, 8, 20, 160));
    ventana.draw(overlay);
}

void Menu::dibujarPrincipal() {
    float W = (float)ANCHO_VENTANA;
    float H = (float)ALTO_VENTANA;

    // Líneas horizontales neón SOLO (sin línea vertical)
    float pulso = 0.5f + 0.5f * std::sin(tiempoTotal * 2.5f);
    sf::Color neon1(0, (sf::Uint8)(180 + 75*pulso), 255, 200);

    dibujarLineaNeon(0, H * 0.12f, W, H * 0.12f, neon1, 1.5f);
    dibujarLineaNeon(0, H * 0.88f, W, H * 0.88f, neon1, 1.5f);

    // Sombra del título
    sf::Text sombra = crearTexto("OVERDRIVE", 96, sf::Color(0,0,0,180),
                                  W*0.5f+4, H*0.22f+4, true);
    ventana.draw(sombra);

    // Glitch sutil
    float glitch = std::sin(tiempoTotal * 7.3f) > 0.95f ? 3.f : 0.f;
    sf::Text titulo = crearTexto("OVERDRIVE", 96,
        sf::Color(255, (sf::Uint8)(255*pulso), 255), W*0.5f+glitch, H*0.22f, true);
    ventana.draw(titulo);

    sf::Text sub = crearTexto("RACE OR DIE", 22, COL_ACENTO, W*0.5f, H*0.33f, true);
    sub.setLetterSpacing(4.f);
    ventana.draw(sub);

    float startY = H * 0.45f, step = 72.f, btnW = 320.f, btnH = 52.f;
    for (int i = 0; i < NUM_OPCIONES; ++i)
        dibujarBoton(OPCIONES[i], W*0.5f - btnW*0.5f, startY + i*step, btnW, btnH,
                     i == opcionActual, COL_ACENTO);

    sf::Text pie = crearTexto("USE  W/S  OR  ARROWS  +  ENTER", 14,
                               COL_DIM, W*0.5f, H*0.93f, true);
    pie.setLetterSpacing(2.f);
    ventana.draw(pie);
}

void Menu::dibujarComoJugar() {
    float W = (float)ANCHO_VENTANA, H = (float)ALTO_VENTANA;
    float pw = 700.f, ph = 460.f;
    float px = (W-pw)*0.5f, py = (H-ph)*0.5f;
    dibujarPanelOscuro(px, py, pw, ph, COL_ACENTO);

    ventana.draw(crearTexto("COMO JUGAR", 42, COL_ACENTO, W*0.5f, py+20.f, true));
    dibujarLineaNeon(px+20, py+72, px+pw-20, py+72, COL_ACENTO, 1.5f);

    float col1x = px+40.f, col2x = px+pw*0.5f+20.f, ry = py+95.f, ls = 36.f;

    ventana.draw(crearTexto("JUGADOR 1", 22, colorJ(0), col1x, ry));
    ry += ls+8;
    struct Ctrl { const char* a; const char* t; };
    Ctrl c1[] = {{"Moverse","A  /  D"},{"Saltar","W"},
                  {"Deslizar","S"},{"Gancho","SHIFT IZQ"}};
    for (auto& c : c1) {
        ventana.draw(crearTexto(c.a, 18, COL_TEXTO, col1x, ry));
        ventana.draw(crearTexto(c.t, 16, COL_DIM, col1x+110, ry));
        ry += ls;
    }

    ry = py+95.f;
    ventana.draw(crearTexto("JUGADOR 2", 22, colorJ(1), col2x, ry));
    ry += ls+8;
    Ctrl c2[] = {{"Moverse","FLECHA IZQ / DER"},{"Saltar","FLECHA ARRIBA"},
                  {"Deslizar","FLECHA ABAJO"},{"Gancho","SHIFT DER"}};
    for (auto& c : c2) {
        ventana.draw(crearTexto(c.a, 18, COL_TEXTO, col2x, ry));
        ventana.draw(crearTexto(c.t, 16, COL_DIM, col2x+110, ry));
        ry += ls;
    }

    float ry2 = py+ph-130.f;
    dibujarLineaNeon(px+20, ry2-10, px+pw-20, ry2-10, COL_ACENTO2, 1.f);
    ventana.draw(crearTexto("MANTEN LA DELANTERA Y SOBREVIVE EN LAS PLATAFORMAS",
                             18, sf::Color(255,200,80), W*0.5f, ry2+5, true));
    ventana.draw(crearTexto("El jugador que quede sin vidas pierde.  3 vidas por jugador.",
                             15, COL_TEXTO, W*0.5f, ry2+32, true));
    ventana.draw(crearTexto("Los picos matan al instante.  Las cajas te frenan.",
                             15, COL_TEXTO, W*0.5f, ry2+55, true));
    ventana.draw(crearTexto("[ ESC  o  ENTER  para volver ]", 16, COL_DIM,
                             W*0.5f, py+ph-28.f, true));
}

void Menu::dibujarAjustes() {
    float W = (float)ANCHO_VENTANA, H = (float)ALTO_VENTANA;
    // Solo UNA opción: Música
    float pw = 400.f, ph = 180.f;
    float px = (W-pw)*0.5f, py = (H-ph)*0.5f;
    dibujarPanelOscuro(px, py, pw, ph, COL_ACENTO2);

    ventana.draw(crearTexto("AJUSTES", 38, COL_ACENTO2, W*0.5f, py+18.f, true));
    dibujarLineaNeon(px+20, py+62, px+pw-20, py+62, COL_ACENTO2, 1.5f);

    float oy = py+85.f;
    // Highlight
    sf::RectangleShape hl({pw-40.f, 40.f});
    hl.setPosition(px+20.f, oy-4);
    hl.setFillColor(sf::Color(0,180,255,30));
    hl.setOutlineColor(COL_ACENTO); hl.setOutlineThickness(1.f);
    ventana.draw(hl);

    ventana.draw(crearTexto("MUSICA", 22, COL_ACENTO, px+40.f, oy));

    // Checkbox
    sf::RectangleShape cbox({24.f,24.f});
    cbox.setPosition(px+pw-70.f, oy-2);
    cbox.setFillColor(sf::Color::Transparent);
    cbox.setOutlineColor(COL_ACENTO); cbox.setOutlineThickness(2.f);
    ventana.draw(cbox);
    if (!musicaMuteada) {
        sf::RectangleShape check({14.f,14.f});
        check.setPosition(px+pw-65.f, oy+3);
        check.setFillColor(COL_ACENTO);
        ventana.draw(check);
    }
    ventana.draw(crearTexto(musicaMuteada ? "MUTED" : "ON", 16,
        musicaMuteada ? COL_DIM : sf::Color(80,255,140), px+pw-38.f, oy+4));

    ventana.draw(crearTexto("ENTER para cambiar  |  ESC para volver",
                             14, COL_DIM, W*0.5f, py+ph-26.f, true));
}

void Menu::dibujarSeleccion() {
    float W = (float)ANCHO_VENTANA, H = (float)ALTO_VENTANA;
    sf::Color colActivo = colorJ(jugadorActivo);

    std::string tit = (jugadorActivo==0)
        ? "JUGADOR 1  >  ELIGE TU PERSONAJE"
        : "JUGADOR 2  >  ELIGE TU PERSONAJE";
    ventana.draw(crearTexto(tit, 28, colActivo, W*0.5f, 22.f, true));
    dibujarLineaNeon(0, 62, W, 62, colActivo, 2.f);

    int n = (int)personajes.size();
    float cardW = 190.f, cardH = 210.f, gap = 16.f;
    float totalW = n*cardW + (n-1)*gap;
    float startX = (W-totalW)*0.5f;
    float cardY  = 75.f;

    int selActual = (jugadorActivo==0) ? seleccionJ1 : seleccionJ2;
    int selOtro   = (jugadorActivo==0) ? seleccionJ2 : seleccionJ1;

    for (int i = 0; i < n; ++i) {
        float cx = startX + i*(cardW+gap);
        bool esSel  = (i == selActual);
        bool esOtro = (i == selOtro);
        float esc = esSel ? 1.06f : 1.f;
        float offY = esSel ? -6.f : 0.f;

        sf::Color borde = esSel  ? colActivo
                        : esOtro ? colorJ(1-jugadorActivo)
                        : COL_DIM;

        sf::RectangleShape card({cardW*esc, cardH*esc});
        card.setPosition(cx-(cardW*esc-cardW)*0.5f, cardY+offY);
        card.setFillColor(esSel ? sf::Color(20,24,50,230) : COL_PANEL);
        card.setOutlineColor(borde);
        card.setOutlineThickness(esSel ? 2.5f : 1.f);
        ventana.draw(card);

        // Victory image como perfil
        if (i < (int)sprPerfiles.size() && texPerfiles[i].getSize().x > 0) {
            sf::Sprite& spr = sprPerfiles[i];
            sf::Vector2u ts = texPerfiles[i].getSize();
            float maxH = cardH*esc - 36.f;
            float maxW = cardW*esc - 8.f;
            float sc = std::min(maxW/(float)ts.x, maxH/(float)ts.y);
            spr.setScale(sc, sc);
            float sx = cx + (cardW*esc - ts.x*sc)*0.5f - (cardW*esc-cardW)*0.5f;
            float sy = cardY + offY + 4.f;
            spr.setPosition(sx, sy);
            spr.setColor(esSel ? sf::Color(255,255,255,255) : sf::Color(110,110,130,200));
            ventana.draw(spr);
        }

        // Alias
        ventana.draw(crearTexto(personajes[i].alias, 14,
            esSel ? colActivo : COL_DIM,
            cx+cardW*0.5f, cardY+cardH*esc-20.f+offY, true));

        // Marcas J1/J2
        if (jugadorActivo==1 && i==seleccionJ1) {
            ventana.draw(crearTexto("J1",12,colorJ(0), cx+cardW*esc-24.f, cardY+3.f+offY));
        }
        if (jugadorActivo==0 && i==seleccionJ2) {
            ventana.draw(crearTexto("J2",12,colorJ(1), cx+cardW*esc-24.f, cardY+3.f+offY));
        }
    }

    // Panel de lore
    float sep = cardY + cardH + 14.f;
    dibujarLineaNeon(0, sep, W, sep, colActivo, 1.5f);

    float loreY = sep + 14.f;
    const DatosPersonaje& dp = personajes[selActual];

    ventana.draw(crearTexto(dp.nombre, 26, colActivo, W*0.5f, loreY, true));

    float ly = loreY + 38.f;
    std::string lore = dp.lore;
    size_t pos = 0;
    while ((pos = lore.find('\n')) != std::string::npos) {
        ventana.draw(crearTexto(lore.substr(0,pos), 17, COL_TEXTO, W*0.5f, ly, true));
        lore = lore.substr(pos+1); ly += 26.f;
    }
    ventana.draw(crearTexto(lore, 17, COL_TEXTO, W*0.5f, ly, true));

    // Instrucciones pie
    float instrY = H - 48.f;
    dibujarLineaNeon(0, instrY-10, W, instrY-10, COL_DIM, 1.f);
    std::string instrStr = (jugadorActivo==0)
        ? "J1:  FLECHAS IZQ/DER para cambiar  |  ENTER para confirmar  |  ESC para volver"
        : "J2:  FLECHAS IZQ/DER para cambiar  |  F o ENTER para confirmar  |  ESC para cancelar";
    ventana.draw(crearTexto(instrStr, 14, COL_DIM, W*0.5f, instrY, true));
}

// ─────────────────────────────────────────────────────────────
// UTILIDADES
// ─────────────────────────────────────────────────────────────
sf::Text Menu::crearTexto(const std::string& str, unsigned size,
                            sf::Color color, float x, float y, bool centrarX) {
    sf::Text t;
    t.setFont(fuente); t.setCharacterSize(size); t.setFillColor(color); t.setString(str);
    if (centrarX) {
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width*0.5f, b.top);
    }
    t.setPosition(x, y);
    return t;
}

void Menu::dibujarPanelOscuro(float x, float y, float w, float h, sf::Color borde) {
    sf::RectangleShape p({w,h});
    p.setPosition(x,y); p.setFillColor(COL_PANEL);
    p.setOutlineColor(borde); p.setOutlineThickness(2.f);
    ventana.draw(p);
}

void Menu::dibujarLineaNeon(float x1, float y1, float x2, float y2,
                              sf::Color color, float grosor) {
    float dx=x2-x1, dy=y2-y1, len=std::sqrt(dx*dx+dy*dy);
    if (len < 1.f) return;
    sf::RectangleShape l({len, grosor});
    l.setPosition(x1,y1); l.setFillColor(color);
    l.setRotation(std::atan2(dy,dx)*180.f/3.14159f);
    ventana.draw(l);
}

void Menu::dibujarBoton(const std::string& txt, float x, float y,
                         float w, float h, bool sel, sf::Color color) {
    sf::RectangleShape btn({w,h});
    btn.setPosition(x,y);
    if (sel) {
        btn.setFillColor(sf::Color(color.r/6,color.g/6,color.b/6,200));
        btn.setOutlineColor(color); btn.setOutlineThickness(2.f);
    } else {
        btn.setFillColor(sf::Color(20,22,40,180));
        btn.setOutlineColor(COL_DIM); btn.setOutlineThickness(1.f);
    }
    ventana.draw(btn);
    if (sel) {
        sf::RectangleShape ac({4.f,h});
        ac.setPosition(x,y); ac.setFillColor(color);
        ventana.draw(ac);
    }
    sf::Text t;
    t.setFont(fuente); t.setCharacterSize(22);
    t.setFillColor(sel ? color : COL_TEXTO);
    t.setString(txt); t.setLetterSpacing(2.f);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left+b.width*0.5f, b.top+b.height*0.5f);
    t.setPosition(x+w*0.5f, y+h*0.5f);
    ventana.draw(t);
}
