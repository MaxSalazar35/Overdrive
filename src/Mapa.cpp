// ============================================================
// Mapa.cpp — Overdrive (Box2D v3)
//
// Diseño limpio:
//   - Suelo continuo siempre (y=620, h=100). Nunca hay huecos
//     en el suelo excepto en el chunk de picos, donde la base
//     roja tapa el hueco.
//   - Plataformas de h FIJA (60px) suspendidas. No bajan al suelo.
//     Se ven como bloques flotantes sobre el fondo cyberpunk.
//   - Rampas conectan dos plataformas de la misma altura (o que 
//     terminen en el mismo y). Base de la rampa = ySup + hMax.
//   - Sin solapamientos: cada rampa ocupa el hueco exacto entre
//     dos plataformas sin tocarlas.
//   - Spawn garantizado en zona libre (antes del primer bloque).
// ============================================================
#include "Mapa.hpp"
#include "Constantes.hpp"
#include <iostream>
#include <cmath>

static constexpr float SUELO_Y  = 620.f;   // y donde empieza el suelo
static constexpr float PLAT_H   = 60.f;    // altura estándar de plataforma
static constexpr float RAMPA_W  = 80.f;    // ancho de una rampa de conexión

static const sf::Color COL_SUELO    (22,  30,  52);
static const sf::Color COL_PLAT     (35,  52,  95);
static const sf::Color COL_PLAT_TOP (120, 190, 255);
static const sf::Color COL_RAMPA    (30,  55,  82);
static const sf::Color COL_RAMPA_TOP(100, 175, 255);
static const sf::Color COL_PICO_BASE(55,  18,  18);
static const sf::Color COL_PICO_PTA (210, 45,  45);

// ─────────────────────────────────────────────────────────────
// PLANTILLA A — Normal: plataformas a diferentes alturas
// Cada plataforma es {x, ySup, w, true/false, h=PLAT_H(default)}
// Las rampas conectan plataformas adyacentes sin solapar.
// ─────────────────────────────────────────────────────────────
// Diseño A: (suelo a y=620)
//  Plat1: x=100..460, y=480   → superficie en 480, base en 540
//  Rampa1: x=460..540, baja de 480 a 400
//  Plat2: x=540..840, y=400
//  Rampa2: x=840..920, sube de 400 a 480
//  Plat3: x=920..1240, y=480
//  ... etc.
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_A = {
    // Suelo continuo
    {    0.f, SUELO_Y, 3840.f, true, 100.f },
    // Plataformas (h por defecto = PLAT_H, calculado en generarConPlantilla)
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
// Rampas que llenan exactamente el hueco entre plataformas
// {x, ySup_alta, w, hIzq, hDer}
// ySup_alta = y de la plataforma más alta de las dos que conecta
// hIzq/hDer = diferencia de altura entre las dos plataformas
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_A = {
    // hIzq siempre 0, hDer=diff, ySup=min(y_izq,y_der) → punta siempre →
    {  460.f, 400.f, 80.f, 0.f,  80.f},  // plat1(480)→plat2(400)
    {  840.f, 400.f, 80.f, 0.f,  80.f},  // plat2(400)→plat3(480)
    { 1240.f, 360.f, 80.f, 0.f, 120.f},  // plat3(480)→plat4(360)
    { 1600.f, 360.f, 80.f, 0.f,  80.f},  // plat4(360)→plat5(440)
    { 2020.f, 360.f, 80.f, 0.f,  80.f},  // plat5(440)→plat6(360)
    { 2400.f, 360.f, 80.f, 0.f, 120.f},  // plat6(360)→plat7(480)
    { 2800.f, 300.f, 80.f, 0.f, 180.f},  // plat7(480)→plat8(300)
    { 3160.f, 300.f, 80.f, 0.f, 100.f},  // plat8(300)→plat9(400)
    { 3560.f, 360.f, 80.f, 0.f,  40.f},  // plat9(400)→plat10(360)
};
const std::vector<Mapa::DefPico> Mapa::PICOS_A = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA B — Descenso: plataformas escalonadas hacia abajo
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_B = {
    {    0.f, SUELO_Y, 3840.f, true, 100.f },
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
    {  420.f, 260.f, 80.f, 0.f,  60.f},  // 260→320
    {  840.f, 320.f, 80.f, 0.f,  60.f},  // 320→380
    { 1240.f, 380.f, 80.f, 0.f,  60.f},  // 380→440
    { 1660.f, 440.f, 80.f, 0.f,  60.f},  // 440→500
    { 2060.f, 500.f, 80.f, 0.f,  40.f},  // 500→540
    { 2440.f, 460.f, 80.f, 0.f,  80.f},  // 540→460
    { 2820.f, 380.f, 80.f, 0.f,  80.f},  // 460→380
    { 3200.f, 300.f, 80.f, 0.f,  80.f},  // 380→300
    { 3560.f, 260.f, 80.f, 0.f,  40.f},  // 300→260
};
const std::vector<Mapa::DefPico> Mapa::PICOS_B = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA C — Altura: plataformas muy altas + techos de gancho
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_C = {
    {    0.f, SUELO_Y, 3840.f, true, 100.f },
    // Plataformas bajas
    {  100.f, 500.f,  240.f, true },
    {  420.f, 460.f,  220.f, true },
    {  720.f, 500.f,  240.f, true },
    { 1040.f, 440.f,  220.f, true },
    // Salto: plataformas altas
    { 1360.f, 260.f,  280.f, true },
    { 1720.f, 200.f,  260.f, true },
    { 2060.f, 260.f,  280.f, true },
    // Techos de gancho (altos, h fijo pequeño para no llenar la pantalla)
    { 1400.f,  80.f,  480.f, true, 40.f },
    { 2100.f,  60.f,  440.f, true, 40.f, true },
    { 2700.f,  80.f,  500.f, true, 40.f, true },
    // Plataformas bajas finales
    { 2440.f, 480.f,  260.f, true },
    { 2800.f, 440.f,  280.f, true },
    { 3160.f, 500.f,  260.f, true },
    { 3520.f, 460.f,  240.f, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_C = {
    {  340.f, 460.f, 80.f, 0.f,  40.f},
    {  640.f, 460.f, 80.f, 0.f,  40.f},
    {  960.f, 440.f, 80.f, 0.f,  60.f},
    { 2380.f, 260.f, 80.f, 0.f, 220.f},
    { 2720.f, 440.f, 80.f, 0.f,  40.f},
    { 3080.f, 440.f, 80.f, 0.f,  60.f},
    { 3440.f, 460.f, 80.f, 0.f,  40.f},
};
const std::vector<Mapa::DefPico> Mapa::PICOS_C = {};

// ─────────────────────────────────────────────────────────────
// PLANTILLA D — Peligro: picos reemplazando parte del suelo
// Los picos ocupan los huecos del suelo. Las plataformas sobre
// los picos son la única forma segura de cruzar.
// ─────────────────────────────────────────────────────────────
const std::vector<Mapa::DefTerreno> Mapa::TERRENO_D = {
    // Suelo con huecos (los picos rellenan los huecos)
    {    0.f, SUELO_Y,  380.f, true, 100.f },
    {  720.f, SUELO_Y,  520.f, true, 100.f },
    { 1580.f, SUELO_Y,  440.f, true, 100.f },
    { 2360.f, SUELO_Y,  540.f, true, 100.f },
    { 3240.f, SUELO_Y,  600.f, true, 100.f },
    // Plataformas sobre los huecos de picos (alcanzables con salto, max 160px sobre suelo)
    {  280.f, 460.f,  280.f, true },
    {  640.f, 480.f,  260.f, true },
    {  960.f, 460.f,  280.f, true },
    { 1300.f, 460.f,  300.f, true },
    { 1660.f, 480.f,  280.f, true },
    { 2020.f, 460.f,  320.f, true },
    { 2420.f, 460.f,  280.f, true },
    { 2760.f, 480.f,  300.f, true },
    { 3140.f, 460.f,  280.f, true },
    { 3500.f, 480.f,  260.f, true },
    // Techos de gancho
    {  460.f,  90.f,  460.f, true, 40.f, true },
    { 1140.f,  70.f,  420.f, true, 40.f, true },
    { 2060.f,  90.f,  460.f, true, 40.f, true },
    { 2900.f,  70.f,  380.f, true, 40.f, true },
};
const std::vector<Mapa::DefRampa> Mapa::RAMPAS_D = {
    // Todas las rampas con hIzq=0 (punta →), ySup=min(y_izq,y_der), hDer=diff
    {  200.f, 460.f, 80.f, 0.f, 20.f},  // suelo(620→460 pero viene de suelo) — rampa de acceso
    {  560.f, 460.f, 80.f, 0.f, 20.f},  // plat1(460)→plat2(480)
    {  880.f, 460.f, 80.f, 0.f, 20.f},  // plat2(480)→plat3(460)
    { 1220.f, 460.f, 80.f, 0.f, 20.f},  // plat3(460)→plat4(460) plano
    { 1580.f, 460.f, 80.f, 0.f, 20.f},  // plat4(460)→plat5(480)
    { 1940.f, 460.f, 80.f, 0.f, 20.f},  // plat5(480)→plat6(460)
    { 2340.f, 460.f, 80.f, 0.f, 20.f},  // plat6(460)→plat7(460) plano
    { 2680.f, 460.f, 80.f, 0.f, 20.f},  // plat7(460)→plat8(480)
    { 3060.f, 460.f, 80.f, 0.f, 20.f},  // plat8(480)→plat9(460)
    { 3420.f, 460.f, 80.f, 0.f, 20.f},  // plat9(460)→plat10(480)
};
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
    // Techo global
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
    // Orden: A → D (picos) → B (descenso) → C (altura) → repite
    // Así los picos aparecen desde el segundo chunk en adelante.
    int tipo = contadorChunk % 4; contadorChunk++;
    const std::vector<DefTerreno>* t; const std::vector<DefRampa>* r;
    const std::vector<DefPico>* p;
    switch(tipo) {
        case 0: t=&TERRENO_A;r=&RAMPAS_A;p=&PICOS_A; break;
        case 1: t=&TERRENO_D;r=&RAMPAS_D;p=&PICOS_D; break;  // picos pronto
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
        // Sin espejado — todos los chunks van en la misma dirección
        float px = d.x;
        float h = (d.h > 0.f) ? d.h : PLAT_H;
        agregarTerreno(ox + px, d.y, d.w, h, d.gancheable, idx, d.esTechoGancho);
    }
    for (const auto& d : rampas) {
        agregarRampa(ox + d.x, d.ySup, d.w, d.hIzq, d.hDer, idx);
    }
    for (const auto& d : picos) {
        agregarPicos(ox + d.x, d.y, d.w, d.cantidad, idx);
    }
    for (const auto& d : cajas_def) {
        agregarCaja(ox + d.x, d.y, idx);
    }
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
    // Calcular chunk actual según posición de la cámara
    // Usamos el chunk donde está la cámara, con offset al inicio del chunk
    float chunkActualX = 0.f;
    if (camaraX > 0.f) {
        int chunkActual = (int)(camaraX / anchoPx);
        // Clamp al rango de chunks vivos
        if (chunkActual < chunkMin) chunkActual = chunkMin;
        if (chunkActual > chunkMax) chunkActual = chunkMax;
        chunkActualX = chunkActual * anchoPx;
    } else {
        chunkActualX = chunkMin * anchoPx;
    }
    // Spawn al inicio del chunk actual, en zona libre antes del primer bloque
    float spawnY = SUELO_Y - 40.f;
    return id == 0 ? sf::Vector2f(chunkActualX + 20.f, spawnY)
                   : sf::Vector2f(chunkActualX + 55.f, spawnY);
}

float Mapa::getAnchoTotal() const { return (chunkMax+1)*anchoPx; }

// ── agregarTerreno ────────────────────────────────────────────
void Mapa::agregarTerreno(float x, float y, float w, float h,
                           bool gancheable, int chunk, bool esTecho)
{
    Plataforma p;
    p.tipo=TipoPlat::Terreno; p.esGancheable=gancheable;
    p.esPico=false; p.chunk=chunk; p.usaConvex=false;
    p.esTechoGancho = esTecho;

    bool esSuelo = (y >= SUELO_Y - 1.f);
    p.rectVisual.setSize({w, h});
    p.rectVisual.setPosition(x, y);
    // Techos de gancho: color diferenciado (cyan/blanco brillante)
    sf::Color colFill  = esSuelo ? COL_SUELO : (esTecho ? sf::Color(20,50,80) : COL_PLAT);
    sf::Color colBorde = esSuelo ? sf::Color(40,55,90,140)
                       : (esTecho ? sf::Color(200,240,255) : COL_PLAT_TOP);
    p.rectVisual.setFillColor(colFill);
    p.rectVisual.setOutlineColor(colBorde);
    p.rectVisual.setOutlineThickness(esSuelo ? 1.f : (esTecho ? 3.f : 2.f));

    b2BodyDef bd = b2DefaultBodyDef(); bd.type = b2_staticBody;
    bd.position = {x + w/2.f, y + h/2.f};
    p.cuerpo = b2CreateBody(worldId, &bd); p.valido = true;
    b2Polygon box = b2MakeBox(w/2.f, h/2.f);
    b2ShapeDef sd = b2DefaultShapeDef(); sd.friction = 0.7f;
    b2CreatePolygonShape(p.cuerpo, &sd, &box);
    plataformas.push_back(p);

    // Franja de acento superior (no en el suelo)
    if (!esSuelo) {
        Plataforma ac;
        ac.tipo=TipoPlat::Terreno; ac.chunk=chunk; ac.usaConvex=false;
        ac.valido=false; ac.esPico=false; ac.esGancheable=false;
        ac.rectVisual.setSize({w, 4.f});
        ac.rectVisual.setPosition(x, y);
        // Techos de gancho: franja blanca muy visible
        ac.rectVisual.setFillColor(esTecho ? sf::Color(220,240,255,255) : sf::Color(100,180,255,200));
        ac.rectVisual.setOutlineThickness(0.f);
        plataformas.push_back(ac);
    }
}

// ── agregarRampa ─────────────────────────────────────────────
// ySup  = y de la esquina más alta del trapecio
// hIzq  = altura del lado izquierdo (0 = punta, >0 = plano)
// hDer  = altura del lado derecho
// yBase = ySup + max(hIzq, hDer)  — coincide con la base de las plataformas adyacentes
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

    p.convVisual.setPointCount(4);
    p.convVisual.setPoint(0, {x,     yBase});
    p.convVisual.setPoint(1, {x + w, yBase});
    p.convVisual.setPoint(2, {x + w, yTopDer});
    p.convVisual.setPoint(3, {x,     yTopIzq});
    p.convVisual.setFillColor(COL_RAMPA);
    p.convVisual.setOutlineColor(sf::Color(65,115,175,180));
    p.convVisual.setOutlineThickness(2.f);

    // Física — trapecio con vértices relativos al centroide
    b2Vec2 vW[4]; int nV = 0;
    vW[nV++] = {x,     yBase};
    if (hDer > 0.f) vW[nV++] = {x + w, yBase};
    vW[nV++] = {x + w, yTopDer};
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
    b2ShapeDef sd = b2DefaultShapeDef(); sd.friction = 0.9f;
    b2CreatePolygonShape(p.cuerpo, &sd, &poly);
    plataformas.push_back(p);

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

    // Base con hitbox de muerte
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
    // Triángulos visuales
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
