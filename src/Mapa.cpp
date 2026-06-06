// ============================================================
// Mapa.cpp — Overdrive (Box2D v3)
//
// CONVENCIÓN DE RAMPAS (definitiva):
//   Bajada (plat izq ALTA → plat der BAJA, jugador desciende):
//     hIzq = diff, hDer = 0  → punta ← (la punta señala de dónde viene)
//     ySup = y_der (la más baja en pantalla = mayor y)
//     yBase = ySup + hIzq = y_der + diff = y_izq  ← conecta con plat izq
//
//   Subida (plat izq BAJA → plat der ALTA, jugador asciende):
//     hIzq = 0, hDer = diff  → punta → (la punta señala hacia dónde va)
//     ySup = y_der (la más alta en pantalla = menor y)
//     yBase = ySup + hDer = y_der + diff = y_izq  ← conecta con plat izq
//
// BLOQUES: h fija (PLAT_H=60px). Sin extensión hasta SUELO_Y para evitar solapamiento con picos.
// ============================================================
#include "Mapa.hpp"
#include "Constantes.hpp"
#include <iostream>
#include <cmath>

static constexpr float SUELO_Y = 620.f;
static constexpr float PLAT_H  = 60.f;

static const sf::Color COL_SUELO    (22,  30,  52);
static const sf::Color COL_PLAT     (35,  52,  95);
static const sf::Color COL_PLAT_TOP (120, 190, 255);
static const sf::Color COL_RAMPA    (30,  55,  82);
static const sf::Color COL_PICO_BASE(55,  18,  18);
static const sf::Color COL_PICO_PTA (210, 45,  45);

// ─────────────────────────────────────────────────────────────
// PLANTILLA A — Normal
// Plataformas: 480,400,480,360,440,360,480,300,400,360
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_A = {
    {    0.f, SUELO_Y, 3840.f, true, 100.f },         // suelo
    {    0.f,    0.f, 3840.f, true,  20.f, true },    // techo continuo gancheable
    {  100.f, 480.f,  360.f, true },
    {  540.f, 400.f,  300.f, true },
    {  920.f, 480.f,  320.f, true },
    { 1320.f, 360.f,  280.f, true },
    { 1680.f, 440.f,  340.f, true },
    { 2100.f, 360.f,  300.f, true },
    { 2480.f, 480.f,  320.f, true },
    { 2880.f, 300.f,  280.f, true },
    { 3240.f, 400.f,  320.f, true },
    { 3640.f, 360.f,  200.f, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_A = {
    // 480→400 SUBIDA:  ySup=400, hIzq=0,   hDer=80  → yTopIzq=480 yTopDer=400
    {  460.f, 400.f, 80.f,   0.f,  80.f},
    // 400→480 BAJADA:  ySup=400, hIzq=80,  hDer=0   → yTopIzq=400 yTopDer=480
    {  840.f, 400.f, 80.f,  80.f,   0.f},
    // 480→360 SUBIDA:  ySup=360, hIzq=0,   hDer=120 → yTopIzq=480 yTopDer=360
    { 1240.f, 360.f, 80.f,   0.f, 120.f},
    // 360→440 BAJADA:  ySup=360, hIzq=80,  hDer=0   → yTopIzq=360 yTopDer=440
    { 1600.f, 360.f, 80.f,  80.f,   0.f},
    // 440→360 SUBIDA:  ySup=360, hIzq=0,   hDer=80  → yTopIzq=440 yTopDer=360
    { 2020.f, 360.f, 80.f,   0.f,  80.f},
    // 360→480 BAJADA:  ySup=360, hIzq=120, hDer=0   → yTopIzq=360 yTopDer=480
    { 2400.f, 360.f, 80.f, 120.f,   0.f},
    // 480→300 SUBIDA:  ySup=300, hIzq=0,   hDer=180 → yTopIzq=480 yTopDer=300
    { 2800.f, 300.f, 80.f,   0.f, 180.f},
    // 300→400 BAJADA:  ySup=300, hIzq=100, hDer=0   → yTopIzq=300 yTopDer=400
    { 3160.f, 300.f, 80.f, 100.f,   0.f},
    // 400→360 SUBIDA:  ySup=360, hIzq=0,   hDer=40  → yTopIzq=400 yTopDer=360
    { 3560.f, 360.f, 80.f,   0.f,  40.f},
};
const std::vector<Mapa::DefPico> Mapa::PICOS_A = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA B — Descenso escalonado
// Plataformas: 260,320,380,440,500,540,460,380,300,260
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_B = {
    {    0.f, SUELO_Y, 3840.f, true, 100.f },         // suelo
    {    0.f,    0.f, 3840.f, true,  20.f, true },    // techo continuo gancheable
    {   60.f, 260.f,  360.f, true },
    {  500.f, 320.f,  340.f, true },
    {  920.f, 380.f,  320.f, true },
    { 1320.f, 440.f,  340.f, true },
    { 1740.f, 500.f,  320.f, true },
    { 2140.f, 540.f,  300.f, true },
    { 2520.f, 460.f,  300.f, true },
    { 2900.f, 380.f,  300.f, true },
    { 3280.f, 300.f,  280.f, true },
    { 3640.f, 260.f,  200.f, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_B = {
    // 260→320 BAJADA:  ySup=260, hIzq=60, hDer=0 → yTopIzq=260 yTopDer=320
    {  420.f, 260.f, 80.f,  60.f,   0.f},
    // 320→380 BAJADA:  ySup=320, hIzq=60, hDer=0 → yTopIzq=320 yTopDer=380
    {  840.f, 320.f, 80.f,  60.f,   0.f},
    // 380→440 BAJADA:  ySup=380, hIzq=60, hDer=0 → yTopIzq=380 yTopDer=440
    { 1240.f, 380.f, 80.f,  60.f,   0.f},
    // 440→500 BAJADA:  ySup=440, hIzq=60, hDer=0 → yTopIzq=440 yTopDer=500
    { 1660.f, 440.f, 80.f,  60.f,   0.f},
    // 500→540 BAJADA:  ySup=500, hIzq=40, hDer=0 → yTopIzq=500 yTopDer=540
    { 2060.f, 500.f, 80.f,  40.f,   0.f},
    // 540→460 SUBIDA:  ySup=460, hIzq=0,  hDer=80 → yTopIzq=540 yTopDer=460
    { 2440.f, 460.f, 80.f,   0.f,  80.f},
    // 460→380 SUBIDA:  ySup=380, hIzq=0,  hDer=80 → yTopIzq=460 yTopDer=380
    { 2820.f, 380.f, 80.f,   0.f,  80.f},
    // 380→300 SUBIDA:  ySup=300, hIzq=0,  hDer=80 → yTopIzq=380 yTopDer=300
    { 3200.f, 300.f, 80.f,   0.f,  80.f},
    // 300→260 SUBIDA:  ySup=260, hIzq=0,  hDer=40 → yTopIzq=300 yTopDer=260
    { 3560.f, 260.f, 80.f,   0.f,  40.f},
};
const std::vector<Mapa::DefPico> Mapa::PICOS_B = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA C — Plataformas altas + techos de gancho
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_C = {
    {    0.f, SUELO_Y, 3840.f, true, 100.f },         // suelo
    {    0.f,    0.f, 3840.f, true,  20.f, true },    // techo continuo gancheable
    {  100.f, 500.f,  240.f, true },
    {  420.f, 460.f,  220.f, true },
    {  720.f, 500.f,  240.f, true },
    { 1040.f, 440.f,  220.f, true },
    { 1360.f, 260.f,  280.f, true },
    { 1720.f, 200.f,  260.f, true },
    { 2060.f, 260.f,  280.f, true },
    // Plataformas bajas finales
    { 2440.f, 480.f,  260.f, true },
    { 2800.f, 440.f,  280.f, true },
    { 3160.f, 500.f,  260.f, true },
    { 3520.f, 460.f,  240.f, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_C = {
    // 500→460 SUBIDA:  ySup=460, hIzq=0,  hDer=40 → yTopIzq=500 yTopDer=460
    {  340.f, 460.f, 80.f,   0.f,  40.f},
    // 460→500 BAJADA:  ySup=460, hIzq=40, hDer=0  → yTopIzq=460 yTopDer=500
    {  640.f, 460.f, 80.f,  40.f,   0.f},
    // 500→440 SUBIDA:  ySup=440, hIzq=0,  hDer=60 → yTopIzq=500 yTopDer=440
    {  960.f, 440.f, 80.f,   0.f,  60.f},
    // Sin rampa entre zona alta (260) y baja (480): desnivel de 220px, se usa gancho
    // 480→440 SUBIDA: ySup=440, hIzq=0, hDer=40, w=100 → yTopIzq=480 yTopDer=440
    { 2700.f, 440.f,100.f,   0.f,  40.f},
    // 440→500 BAJADA:  ySup=440, hIzq=60, hDer=0  → yTopIzq=440 yTopDer=500
    { 3080.f, 440.f, 80.f,  60.f,   0.f},
    // 500→460 SUBIDA:  ySup=460, hIzq=0,  hDer=40 → yTopIzq=500 yTopDer=460
    { 3440.f, 460.f, 80.f,   0.f,  40.f},
};
const std::vector<Mapa::DefPico> Mapa::PICOS_C = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA D — Picos en el suelo, plataformas alcanzables
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_D = {
    {    0.f, SUELO_Y,  380.f, true, 100.f },         // suelo (zona izq)
    {  720.f, SUELO_Y,  520.f, true, 100.f },
    { 1580.f, SUELO_Y,  440.f, true, 100.f },
    { 2360.f, SUELO_Y,  540.f, true, 100.f },
    { 3240.f, SUELO_Y,  600.f, true, 100.f },
    {    0.f,    0.f, 3840.f, true,  20.f, true },    // techo continuo gancheable
    // Plataformas suspendidas: noExtender=true para no solaparse con los picos de abajo
    {  280.f, 460.f,  280.f, true, 0.f, false, true },
    {  640.f, 480.f,  260.f, true, 0.f, false, true },
    {  960.f, 460.f,  280.f, true, 0.f, false, true },
    { 1300.f, 460.f,  300.f, true, 0.f, false, true },
    { 1660.f, 480.f,  280.f, true, 0.f, false, true },
    { 2020.f, 460.f,  320.f, true, 0.f, false, true },
    { 2420.f, 460.f,  280.f, true, 0.f, false, true },
    { 2760.f, 480.f,  300.f, true, 0.f, false, true },
    { 3140.f, 460.f,  280.f, true, 0.f, false, true },
    { 3500.f, 480.f,  260.f, true, 0.f, false, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_D = {};
const std::vector<Mapa::DefPico> Mapa::PICOS_D = {
    {  380.f, SUELO_Y,  340.f, 9 },
    { 1240.f, SUELO_Y,  340.f, 9 },
    { 2100.f, SUELO_Y,  260.f, 7 },
    { 2900.f, SUELO_Y,  340.f, 9 },
};

const std::vector<Mapa::DefCaja> Mapa::PLANTILLA_CAJA = {
    {  280.f, 400.f}, {  640.f, 320.f},
    { 1020.f, 400.f}, { 1400.f, 280.f},
    { 1760.f, 380.f}, { 2160.f, 280.f},
    { 2560.f, 400.f}, { 2960.f, 220.f},
    { 3320.f, 320.f}, { 3660.f, 280.f},
};

// ── Constructor ──────────────────────────────────────────────
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
        } else { modoFallback = true; }
    }
    {
        b2BodyDef bd = b2DefaultBodyDef(); bd.type = b2_staticBody;
        bd.position = {0.f, -20.f};
        b2BodyId c = b2CreateBody(worldId, &bd);
        b2Polygon box = b2MakeBox(500000.f, 10.f);
        b2ShapeDef sd = b2DefaultShapeDef();
        b2CreatePolygonShape(c, &sd, &box);
        Plataforma p; p.cuerpo = c; p.valido = true; p.chunk = -999;
        plataformas.push_back(p);
    }
    generarChunk(0);
    generarChunk(1);
}

Mapa::~Mapa() { destruirFisicas(); }

void Mapa::generarChunk(int idx)
{
    int tipo = contadorChunk % 4; contadorChunk++;
    const std::vector<DefTerreno>* t; const std::vector<DefRampa>* r;
    const std::vector<DefPico>* p;
    switch(tipo) {
        case 0: t=&TERRENO_A;r=&RAMPAS_A;p=&PICOS_A; break;
        case 1: t=&TERRENO_D;r=&RAMPAS_D;p=&PICOS_D; break;
        case 2: t=&TERRENO_B;r=&RAMPAS_B;p=&PICOS_B; break;
        default:t=&TERRENO_C;r=&RAMPAS_C;p=&PICOS_C; break;
    }
    generarConPlantilla(idx,*t,*r,*p,PLANTILLA_CAJA);
}

void Mapa::generarConPlantilla(int idx,
    const std::vector<DefTerreno>& terrenos,
    const std::vector<DefRampa>&   rampas,
    const std::vector<DefPico>&    picos,
    const std::vector<DefCaja>&    cajas_def)
{
    float ox = idx * anchoPx;
    for (const auto& d : terrenos) {
        float h = (d.h > 0.f) ? d.h : PLAT_H;
        agregarTerreno(ox + d.x, d.y, d.w, h, d.gancheable, idx,
                       d.esTechoGancho, d.noExtender);
    }
    for (const auto& d : rampas)
        agregarRampa(ox + d.x, d.ySup, d.w, d.hIzq, d.hDer, idx);
    for (const auto& d : picos)
        agregarPicos(ox + d.x, d.y, d.w, d.cantidad, idx);
    for (const auto& d : cajas_def)
        agregarCaja(ox + d.x, d.y, idx);
}

void Mapa::eliminarChunk(int idx)
{
    plataformas.erase(
        std::remove_if(plataformas.begin(), plataformas.end(),
            [&](Plataforma& p) {
                if (p.chunk == idx) {
                    if (p.valido && b2Body_IsValid(p.cuerpo))
                        b2DestroyBody(p.cuerpo);
                    return true;
                }
                return false;
            }), plataformas.end());
    cajas.erase(
        std::remove_if(cajas.begin(), cajas.end(),
            [&](const CajaItem& c) { return c.chunk == idx; }),
        cajas.end());
}

void Mapa::update(float dt, float camaraX)
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
    if (camaraX + (float)ANCHO_VENTANA > (chunkMax+1)*anchoPx - anchoPx*0.5f)
        { chunkMax++; generarChunk(chunkMax); }
    if (camaraX - (float)ANCHO_VENTANA > (chunkMin+1)*anchoPx)
        { eliminarChunk(chunkMin); chunkMin++; }
}

void Mapa::drawFondo(sf::RenderWindow& v, float camaraX)
{
    if (!modoFallback) {
        for (int i = 0; i < 3; ++i) {
            if (!capaCargada[i]) continue;
            float offset = camaraX * velParallax[i];
            int   texW   = (int)texCapas[i].getSize().x;
            sprCapas[i].setTextureRect(sf::IntRect((int)offset%texW,0,(int)anchoPx,(int)altoPx));
            sprCapas[i].setPosition(camaraX - ANCHO_VENTANA/2.f, 0.f);
            v.draw(sprCapas[i]);
        }
    } else {
        sf::RectangleShape fondo({anchoPx*(chunkMax-chunkMin+1), altoPx});
        fondo.setPosition(chunkMin*anchoPx, 0.f);
        fondo.setFillColor(sf::Color(10,12,30));
        v.draw(fondo);
    }
}

void Mapa::drawPlataformas(sf::RenderWindow& v) {
    for (auto& p : plataformas) {
        if (p.usaConvex) v.draw(p.convVisual);
        else             v.draw(p.rectVisual);
    }
}
void Mapa::drawItems(sf::RenderWindow& v) {
    for (auto& c : cajas) if (c.activa) v.draw(c.visual);
}

sf::Vector2f Mapa::spawnJugador(int id, float camaraX) const {
    float chunkActualX = chunkMin * anchoPx;
    if (camaraX > 0.f) {
        int ch = (int)(camaraX / anchoPx);
        if (ch < chunkMin) ch = chunkMin;
        if (ch > chunkMax) ch = chunkMax;
        chunkActualX = ch * anchoPx;
    }
    float spawnY = SUELO_Y - 40.f;
    return id == 0 ? sf::Vector2f(chunkActualX + 20.f, spawnY)
                   : sf::Vector2f(chunkActualX + 55.f, spawnY);
}

float Mapa::getAnchoTotal() const { return (chunkMax+1)*anchoPx; }

// ── agregarTerreno ────────────────────────────────────────────
// h fija — no se extiende hasta SUELO_Y para evitar solapamiento con picos.
void Mapa::agregarTerreno(float x, float y, float w, float h,
                           bool gancheable, int chunk,
                           bool esTecho, bool noExtender)
{
    bool esSuelo = (y >= SUELO_Y - 1.f);
    // Extender plataformas suspendidas hasta el suelo para que no queden huecas,
    // excepto cuando noExtender=true (chunk D: hay picos debajo) o es techo.
    float hFinal = h;
    if (!esSuelo && !esTecho && !noExtender) {
        float extendida = SUELO_Y - y;
        if (extendida > h) hFinal = extendida;
    }

    Plataforma p;
    p.tipo=TipoPlat::Terreno; p.esGancheable=gancheable;
    p.esPico=false; p.chunk=chunk; p.usaConvex=false;
    p.esTechoGancho = esTecho;

    sf::Color colFill  = esSuelo ? COL_SUELO : (esTecho ? sf::Color(20,50,80) : COL_PLAT);
    sf::Color colBorde = esSuelo ? sf::Color(40,55,90,140)
                       : (esTecho ? sf::Color(200,240,255) : COL_PLAT_TOP);

    p.rectVisual.setSize({w, hFinal});
    p.rectVisual.setPosition(x, y);
    p.rectVisual.setFillColor(colFill);
    p.rectVisual.setOutlineColor(colBorde);
    p.rectVisual.setOutlineThickness(esSuelo ? 1.f : (esTecho ? 3.f : 2.f));

    b2BodyDef bd = b2DefaultBodyDef(); bd.type = b2_staticBody;
    bd.position = {x + w/2.f, y + hFinal/2.f};
    p.cuerpo = b2CreateBody(worldId, &bd); p.valido = true;
    b2Polygon box = b2MakeBox(w/2.f, hFinal/2.f);
    b2ShapeDef sd = b2DefaultShapeDef(); sd.friction = 0.7f;
    b2CreatePolygonShape(p.cuerpo, &sd, &box);
    plataformas.push_back(p);

    // Franja de acento superior
    if (!esSuelo) {
        Plataforma ac;
        ac.tipo=TipoPlat::Terreno; ac.chunk=chunk; ac.usaConvex=false;
        ac.valido=false; ac.esPico=false; ac.esGancheable=false;
        ac.rectVisual.setSize({w, 4.f});
        ac.rectVisual.setPosition(x, y);
        ac.rectVisual.setFillColor(esTecho ? sf::Color(220,240,255,255)
                                           : sf::Color(100,180,255,200));
        ac.rectVisual.setOutlineThickness(0.f);
        plataformas.push_back(ac);
    }
}

// ── agregarRampa ─────────────────────────────────────────────
// Rampa con h fija — sin relleno inferior.
void Mapa::agregarRampa(float x, float ySup, float w,
                         float hIzq, float hDer, int chunk)
{
    if (w <= 0.f) return;
    float hMax = std::max(hIzq, hDer);
    if (hMax < 1.f) return;

    float yBase   = ySup + hMax;
    float yTopIzq = yBase - hIzq;
    float yTopDer = yBase - hDer;

    Plataforma p;
    p.tipo=TipoPlat::Rampa; p.esGancheable=true; p.esPico=false;
    p.chunk=chunk; p.usaConvex=true; p.valido=true;

    // Visual del triángulo/trapecio
    p.convVisual.setPointCount(4);
    p.convVisual.setPoint(0, {x,     yBase});
    p.convVisual.setPoint(1, {x + w, yBase});
    p.convVisual.setPoint(2, {x + w, yTopDer});
    p.convVisual.setPoint(3, {x,     yTopIzq});
    p.convVisual.setFillColor(COL_RAMPA);
    p.convVisual.setOutlineColor(sf::Color(65,115,175,180));
    p.convVisual.setOutlineThickness(2.f);

    // Física
    // Para rampas de SUBIDA (hIzq==0): bajar el vértice físico derecho 3px.
    // Elimina el "piquito" donde la cima de la rampa choca con el borde
    // vertical de la plataforma siguiente. El visual no se toca.
    static constexpr float MARGEN_SUBIDA = 3.f;
    float yTopDerFis = (hIzq == 0.f && hDer > 0.f)
                       ? yTopDer + MARGEN_SUBIDA   // subida: bajar cima derecha
                       : yTopDer;                  // bajada: sin cambio

    b2Vec2 vW[4]; int nV = 0;
    vW[nV++] = {x,     yBase};
    if (hDer > 0.f) vW[nV++] = {x + w, yBase};
    vW[nV++] = {x + w, yTopDerFis};
    if (hIzq > 0.f) vW[nV++] = {x,     yTopIzq};

    float cx = 0.f, cy = 0.f;
    for (int i = 0; i < nV; i++) { cx += vW[i].x; cy += vW[i].y; }
    cx /= nV; cy /= nV;

    b2Vec2 vR[4];
    for (int i = 0; i < nV; i++)
        vR[i] = {vW[nV-1-i].x - cx, vW[nV-1-i].y - cy};

    b2Hull hull = b2ComputeHull(vR, nV);
    if (hull.count < 3) { p.valido = false; plataformas.push_back(p); return; }
    b2Polygon poly = b2MakePolygon(&hull, 0.f);

    b2BodyDef bd = b2DefaultBodyDef(); bd.type = b2_staticBody;
    bd.position = {cx, cy};
    p.cuerpo = b2CreateBody(worldId, &bd);
    // Fricción alta: las rampas se comportan como plataforma normal, no resbalan
    b2ShapeDef sd = b2DefaultShapeDef(); sd.friction = 1.8f;
    b2CreatePolygonShape(p.cuerpo, &sd, &poly);
    plataformas.push_back(p);

    // Sin relleno debajo de la rampa (bloques de h fija, sin solapamiento con picos)

    // Línea de acento en la superficie inclinada
    Plataforma ln;
    ln.tipo=TipoPlat::Rampa; ln.chunk=chunk; ln.usaConvex=true;
    ln.valido=false; ln.esPico=false; ln.esGancheable=false;
    float g = 4.f;
    ln.convVisual.setPointCount(4);
    ln.convVisual.setPoint(0, {x,     yTopIzq});
    ln.convVisual.setPoint(1, {x + w, yTopDer});
    ln.convVisual.setPoint(2, {x + w, yTopDer + g});
    ln.convVisual.setPoint(3, {x,     yTopIzq + g});
    ln.convVisual.setFillColor(sf::Color(110,190,255,200));
    ln.convVisual.setOutlineThickness(0.f);
    plataformas.push_back(ln);
}

// ── agregarPicos ─────────────────────────────────────────────
void Mapa::agregarPicos(float x, float y, float w, int cantidad, int chunk)
{
    if (cantidad <= 0 || w <= 0.f) return;
    float ad = w / (float)cantidad;
    float alt = 36.f;

    {
        Plataforma base;
        base.tipo=TipoPlat::Pico; base.esPico=true; base.esGancheable=false;
        base.chunk=chunk; base.usaConvex=false; base.valido=true;
        float bh = 20.f;
        base.rectVisual.setSize({w, bh});
        base.rectVisual.setPosition(x, y);
        base.rectVisual.setFillColor(COL_PICO_BASE);
        base.rectVisual.setOutlineThickness(0.f);
        b2BodyDef bd = b2DefaultBodyDef(); bd.type = b2_staticBody;
        bd.position = {x + w/2.f, y + bh/2.f};
        base.cuerpo = b2CreateBody(worldId, &bd);
        b2Polygon box = b2MakeBox(w/2.f, bh/2.f);
        b2ShapeDef sd = b2DefaultShapeDef(); sd.friction = 0.f;
        b2CreatePolygonShape(base.cuerpo, &sd, &box);
        plataformas.push_back(base);
    }
    for (int i = 0; i < cantidad; ++i) {
        float dx = x + i * ad;
        Plataforma pk;
        pk.tipo=TipoPlat::Pico; pk.esPico=false; pk.esGancheable=false;
        pk.chunk=chunk; pk.usaConvex=true; pk.valido=false;
        pk.convVisual.setPointCount(3);
        pk.convVisual.setPoint(0, {dx,           y});
        pk.convVisual.setPoint(1, {dx + ad,       y});
        pk.convVisual.setPoint(2, {dx + ad/2.f,   y - alt});
        pk.convVisual.setFillColor(COL_PICO_PTA);
        pk.convVisual.setOutlineColor(sf::Color(255,100,80,150));
        pk.convVisual.setOutlineThickness(1.5f);
        plataformas.push_back(pk);
    }
}

void Mapa::agregarCaja(float x, float y, int chunk)
{
    CajaItem c; c.posBase={x,y}; c.activa=true; c.chunk=chunk;
    c.visual.setSize({30.f,30.f}); c.visual.setOrigin(15.f,15.f);
    c.visual.setPosition(x,y);
    c.visual.setFillColor(sf::Color(220,180,50));
    c.visual.setOutlineColor(sf::Color(255,220,80));
    c.visual.setOutlineThickness(2.f);
    cajas.push_back(c);
}

void Mapa::resetear()
{
    destruirFisicas(); cajas.clear();
    chunkMin=0; chunkMax=1; contadorChunk=0;
    {
        b2BodyDef bd=b2DefaultBodyDef(); bd.type=b2_staticBody;
        bd.position={0.f,-20.f};
        b2BodyId c=b2CreateBody(worldId,&bd);
        b2Polygon box=b2MakeBox(500000.f,10.f);
        b2ShapeDef sd=b2DefaultShapeDef();
        b2CreatePolygonShape(c,&sd,&box);
        Plataforma p; p.cuerpo=c; p.valido=true; p.chunk=-999;
        plataformas.push_back(p);
    }
    generarChunk(0); generarChunk(1);
}

void Mapa::destruirFisicas() {
    for (auto& p : plataformas)
        if (p.valido && b2Body_IsValid(p.cuerpo))
            b2DestroyBody(p.cuerpo);
    plataformas.clear();
}
