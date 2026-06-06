#pragma once

// ============================================================
// Constantes.hpp — Overdrive
// Todos los valores ajustables del juego en un solo lugar.
// Adaptado para Box2D v3
// ============================================================

// ── Ventana ──────────────────────────────────────────────────
constexpr int   ANCHO_VENTANA   = 1280;
constexpr int   ALTO_VENTANA    = 720;
constexpr int   FPS_LIMITE      = 60;

// ── Física (Box2D v3) ────────────────────────────────────────
constexpr float GRAVEDAD        = 2450.0f;
constexpr float TIEMPO_PASO     = 1.0f / 60.0f;
constexpr int   SUB_PASOS       = 4;       // Box2D v3 usa sub-pasos en lugar de iteraciones

// ── Jugador ──────────────────────────────────────────────────
constexpr float JUG_VEL_MAX        = 480.0f;
constexpr float JUG_ACELERACION    = 1800.0f;
constexpr float JUG_FUERZA_SALTO   = -700.0f;
constexpr float JUG_FRICCION_SUELO = 0.85f;
constexpr float JUG_FRICCION_AIRE  = 0.995f;
constexpr float JUG_ANCHO         = 40.0f;
constexpr float JUG_ALTO          = 70.0f;

// ── Gancho ───────────────────────────────────────────────────
// Longitud del rayo diagonal (px). A 70° alcanza el techo desde cualquier plataforma.
constexpr float GANCHO_LONGITUD       = 580.0f;
// Ángulo del disparo en radianes desde horizontal (70° = muy vertical pero con sesgo)
constexpr float GANCHO_ANGULO         = 1.3963f;   // 80° en rad
// Umbral Y máximo del punto de contacto (solo el techo continuo en y=0..20)
constexpr float GANCHO_Y_MAX_CONTACTO = 25.0f;
// Velocidad de retracción: px/s que se acorta la cuerda cada segundo
constexpr float GANCHO_RETRACCION     = 280.0f;
// Longitud mínima (el jugador queda a esta distancia del techo antes de soltarse)
constexpr float GANCHO_LONG_MIN       = 40.0f;
// Parámetros del joint de distancia (spring)
constexpr float GANCHO_FREQ           = 6.0f;
constexpr float GANCHO_DAMP           = 0.8f;

// ── Cámara ───────────────────────────────────────────────────
constexpr float CAM_VELOCIDAD   = 6.5f;
constexpr float CAM_MARGEN_ELIM = 80.0f;
constexpr float CAM_OFFSET_X    = 0.20f;

// ── Items ────────────────────────────────────────────────────
constexpr float ITEM_RESPAWN    = 9.0f;

// ── Rondas ───────────────────────────────────────────────────
constexpr int   RONDAS_GANAR         = 3;
constexpr float PAUSA_ENTRE_RONDAS   = 3.0f;

// ── Sprites (spritesheet 1896x848, celdas 316x212) ──────────
constexpr int   FRAME_W         = 316;
constexpr int   FRAME_H         = 212;
constexpr int   FILA_IDLE       = 0;
constexpr int   FILA_CORRER     = 1;
constexpr int   FILA_SALTAR     = 2;
constexpr int   FILA_DESLIZAR   = 3;
constexpr int   FRAMES_IDLE     = 4;
constexpr int   FRAMES_CORRER   = 6;
constexpr int   FRAMES_SALTAR   = 4;
constexpr int   FRAMES_DESLIZAR = 2;
constexpr float FT_IDLE         = 0.18f;
constexpr float FT_CORRER       = 0.085f;
constexpr float FT_SALTAR       = 0.11f;
constexpr float FT_DESLIZAR     = 0.14f;

// ── Gamepad ──────────────────────────────────────────────────
constexpr unsigned int BTN_A    = 0;
constexpr unsigned int BTN_B    = 1;
constexpr unsigned int BTN_LB   = 4;
constexpr unsigned int BTN_RB   = 5;
constexpr unsigned int BTN_START= 7;
constexpr float        STICK_DEAD = 15.0f;
