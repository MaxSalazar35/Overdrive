// ============================================================
// Camara.cpp — Overdrive
// Sin límite derecho (mapa infinito). El límite izquierdo
// sigue siendo x = mitad de pantalla (no se puede retroceder).
// ============================================================
#include "Camara.hpp"
#include <algorithm>
#include <limits>

Camara::Camara(float anchoMundo, float altoMundo)
    : anchoMundo(anchoMundo), altoMundo(altoMundo), xObjetivo(ANCHO_VENTANA / 2.f)
{
    vista.setSize((float)ANCHO_VENTANA, (float)ALTO_VENTANA);
    vista.setCenter(ANCHO_VENTANA / 2.f, altoMundo / 2.f);
}

void Camara::update(const std::vector<sf::Vector2f>& posiciones, float dt)
{
    if (posiciones.empty()) return;

    int   lider = idLider(posiciones);
    float xL    = posiciones[lider].x;

    float xMeta   = xL + ANCHO_VENTANA * (0.5f - CAM_OFFSET_X);
    float xActual = vista.getCenter().x;
    float xNueva  = xActual + (xMeta - xActual) * CAM_VELOCIDAD * dt;

    // Solo clamp izquierdo — derecho es infinito
    float mitad = ANCHO_VENTANA / 2.f;
    xNueva = std::max(xNueva, mitad);
    // Sin clamp derecho: el mapa se genera infinitamente

    vista.setCenter(xNueva, altoMundo / 2.f);
}

void Camara::aplicar(sf::RenderWindow& ventana) {
    ventana.setView(vista);
}

void Camara::restaurarVista(sf::RenderWindow& ventana) {
    ventana.setView(ventana.getDefaultView());
}

bool Camara::fueraDePantalla(sf::Vector2f pos) const {
    float bordeIzq   = vista.getCenter().x - ANCHO_VENTANA / 2.f;
    float bordeArriba = vista.getCenter().y - ALTO_VENTANA / 2.f;
    float bordeAbajo  = vista.getCenter().y + ALTO_VENTANA / 2.f;

    bool fueraIzq    = pos.x < bordeIzq   - CAM_MARGEN_ELIM;
    bool fueraArriba = pos.y < bordeArriba - 200.f;
    bool fueraAbajo  = pos.y > bordeAbajo  + 100.f;
    return fueraIzq || fueraArriba || fueraAbajo;
}

sf::FloatRect Camara::getVistaMundo() const {
    sf::Vector2f c = vista.getCenter();
    return {c.x - ANCHO_VENTANA/2.f, c.y - ALTO_VENTANA/2.f,
            (float)ANCHO_VENTANA, (float)ALTO_VENTANA};
}

int Camara::idLider(const std::vector<sf::Vector2f>& pos) const {
    int   mejor = 0;
    float maxX  = std::numeric_limits<float>::lowest();
    for (int i = 0; i < (int)pos.size(); ++i) {
        if (pos[i].x > maxX) { maxX = pos[i].x; mejor = i; }
    }
    return mejor;
}
