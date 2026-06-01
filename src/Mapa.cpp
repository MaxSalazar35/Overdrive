// ============================================================
// Mapa.cpp — Overdrive (Box2D v3)
// ============================================================
#include "Mapa.hpp"
#include "Constantes.hpp"
#include <iostream>
#include <cmath>

Mapa::Mapa(b2WorldId worldId) : worldId(worldId)
{
    const char* rutas[3] = {
        "assets/images/bg_layer1.png",
        "assets/images/bg_layer2.png",
        "assets/images/bg_layer3.png"
    };
    for (int i = 0; i < 3; ++i) {
        if (texCapas[i].loadFromFile(rutas[i])) {
            texCapas[i].setRepeated(true);
            sprCapas[i].setTexture(texCapas[i]);
            capaCargada[i] = true;
        } else {
            std::cerr << "[Mapa] No se cargo " << rutas[i] << "\n";
            modoFallback = true;
        }
    }
    construirNivel();
}

Mapa::~Mapa() { destruirFisicas(); }

void Mapa::construirNivel()
{
    float W = anchoPx;
    float H = altoPx;
    float T = 20.f;   // grosor de los bordes

    // ── Bordes físicos del mundo ──────────────────────────────
    // Techo — impide que los jugadores vuelen infinito
    agregarBorde(0.f,    -T,  W, T);
    // Pared izquierda
    agregarBorde(-T,     0.f, T, H);
    // Pared derecha
    agregarBorde(W,      0.f, T, H);

    // ── Suelo ────────────────────────────────────────────────
    agregarPlataforma(0.f, 670.f, 3840.f, 40.f);
    // Sección 1
    agregarPlataforma(150.f,  540.f, 200.f, 20.f);
    agregarPlataforma(420.f,  460.f, 180.f, 20.f);
    agregarPlataforma(660.f,  530.f, 160.f, 20.f);
    agregarPlataforma(860.f,  400.f, 220.f, 20.f);
    agregarPlataforma(350.f,  600.f, 120.f, 18.f, true);
    // Sección 2
    agregarPlataforma(1000.f, 470.f, 200.f, 20.f);
    agregarPlataforma(1240.f, 370.f, 180.f, 20.f);
    agregarPlataforma(1480.f, 450.f, 160.f, 20.f);
    agregarPlataforma(1700.f, 330.f, 240.f, 20.f);
    agregarPlataforma(1870.f, 430.f, 140.f, 20.f);
    agregarPlataforma(1100.f, 570.f, 150.f, 18.f, true);
    agregarPlataforma(1580.f, 550.f, 110.f, 18.f, true);
    // Sección 3
    agregarPlataforma(2000.f, 490.f, 200.f, 20.f);
    agregarPlataforma(2220.f, 380.f, 180.f, 20.f);
    agregarPlataforma(2460.f, 460.f, 200.f, 20.f);
    agregarPlataforma(2680.f, 350.f, 220.f, 20.f);
    agregarPlataforma(2900.f, 430.f, 160.f, 20.f);
    agregarPlataforma(2100.f, 580.f, 130.f, 18.f, true);
    // Sección 4
    agregarPlataforma(3000.f, 400.f, 200.f, 20.f);
    agregarPlataforma(3220.f, 310.f, 180.f, 20.f);
    agregarPlataforma(3450.f, 390.f, 200.f, 20.f);
    agregarPlataforma(3680.f, 300.f, 180.f, 20.f);
    agregarPlataforma(3100.f, 560.f, 120.f, 18.f, true);

    // Cajas
    agregarCaja(380.f,  420.f); agregarCaja(750.f,  360.f);
    agregarCaja(1060.f, 430.f); agregarCaja(1500.f, 410.f);
    agregarCaja(1750.f, 290.f); agregarCaja(2080.f, 450.f);
    agregarCaja(2500.f, 420.f); agregarCaja(2740.f, 310.f);
    agregarCaja(3080.f, 360.f); agregarCaja(3500.f, 350.f);
}

void Mapa::agregarBorde(float x, float y, float w, float h)
{
    // Borde invisible — solo física, sin visual
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = {x + w/2.f, y + h/2.f};
    b2BodyId cuerpo = b2CreateBody(worldId, &bd);

    b2Polygon box = b2MakeBox(w/2.f, h/2.f);
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.friction   = 0.f;
    b2CreatePolygonShape(cuerpo, &sd, &box);

    // Guardar para destruir después (sin visual)
    Plataforma p;
    p.cuerpo = cuerpo;
    p.valido = true;
    plataformas.push_back(p);
}

void Mapa::agregarPlataforma(float x,float y,float w,float h,bool rampa)
{
    Plataforma p;
    p.visual.setSize({w, h});
    p.visual.setPosition(x, y);
    p.visual.setFillColor(rampa ? sf::Color(50,100,70) : sf::Color(40,60,100));
    p.visual.setOutlineColor(sf::Color(80,120,180,160));
    p.visual.setOutlineThickness(1.5f);

    b2BodyDef bd  = b2DefaultBodyDef();
    bd.type       = b2_staticBody;
    bd.position   = {x + w/2.f, y + h/2.f};
    p.cuerpo      = b2CreateBody(worldId, &bd);
    p.valido      = true;

    b2Polygon box  = b2MakeBox(w/2.f, h/2.f);
    b2ShapeDef sd  = b2DefaultShapeDef();
    sd.friction    = rampa ? 0.05f : 0.6f;
    b2CreatePolygonShape(p.cuerpo, &sd, &box);

    plataformas.push_back(p);
}

void Mapa::agregarCaja(float x,float y)
{
    CajaItem c;
    c.posBase = {x, y};
    c.activa  = true;
    c.visual.setSize({30.f, 30.f});
    c.visual.setOrigin(15.f, 15.f);
    c.visual.setPosition(x, y);
    c.visual.setFillColor(sf::Color(220,180,50));
    c.visual.setOutlineColor(sf::Color(255,220,80));
    c.visual.setOutlineThickness(2.f);
    cajas.push_back(c);
}

void Mapa::update(float dt)
{
    for (auto& c : cajas) {
        if (c.activa) {
            float t = c.reloj.getElapsedTime().asSeconds();
            c.visual.setPosition(c.posBase.x, c.posBase.y + std::sin(t*2.5f)*5.f);
            c.visual.setRotation(t * 25.f);
        } else {
            c.timerRespawn += dt;
            if (c.timerRespawn >= ITEM_RESPAWN) {
                c.activa = true; c.timerRespawn = 0.f; c.reloj.restart();
            }
        }
    }
}

void Mapa::drawFondo(sf::RenderWindow& v, float camaraX)
{
    if (!modoFallback) {
        for (int i = 0; i < 3; ++i) {
            if (!capaCargada[i]) continue;
            float offset = camaraX * velParallax[i];
            int   texW   = (int)texCapas[i].getSize().x;
            sprCapas[i].setTextureRect(
                sf::IntRect((int)offset % texW, 0, (int)anchoPx, (int)altoPx));
            sprCapas[i].setPosition(camaraX - ANCHO_VENTANA/2.f, 0.f);
            v.draw(sprCapas[i]);
        }
    } else {
        sf::RectangleShape fondo({anchoPx, altoPx});
        fondo.setPosition(0.f, 0.f);
        fondo.setFillColor(sf::Color(10,12,30));
        v.draw(fondo);
    }
}

void Mapa::drawPlataformas(sf::RenderWindow& v) {
    for (auto& p : plataformas) v.draw(p.visual);
}
void Mapa::drawItems(sf::RenderWindow& v) {
    for (auto& c : cajas) if (c.activa) v.draw(c.visual);
}
sf::Vector2f Mapa::spawnJugador(int id) const {
    return id == 0 ? sf::Vector2f(120.f, 580.f) : sf::Vector2f(220.f, 580.f);
}
void Mapa::destruirFisicas() {
    for (auto& p : plataformas)
        if (p.valido && b2Body_IsValid(p.cuerpo))
            b2DestroyBody(p.cuerpo);
    plataformas.clear();
}
