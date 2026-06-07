# Overdrive

Juego de plataformas multijugador de velocidad para 2 jugadores, inspirado en SpeedRunners. Desarrollado en **C++ con SFML y Box2D**, compilado con **MinGW64 (MSYS2)** en Visual Studio Code.

Corre, usa el gancho, deja atrás a tu rival y que no te quede fuera de la pantalla.

---

## 🛠️ Prerequisitos

- [Herramientas (VSCode, MSYS2, Git)](./docs/herramientas.md)
- [Extensiones de VSCode](./docs/extensiones.md)
- [Librerías SFML + Box2D](./docs/librerias.md)
- [Clonar el repositorio](./docs/fork.md)

---

## 🗂️ Estructura del Proyecto

```
Overdrive/
├── .github/workflows/publish.yml   ← CI/CD para CETUS
├── src/
│   ├── main.cpp        ← Game loop principal
│   ├── Jugador.cpp     ← Física, animaciones, entrada
│   ├── Gancho.cpp      ← Grappling hook con raycast
│   ├── Camara.cpp      ← Cámara dinámica
│   ├── Mapa.cpp        ← Nivel, plataformas, parallax
│   └── HUD.cpp         ← UI, velocímetro, victoria
├── include/
│   ├── Constantes.hpp  ← Todos los valores del juego
│   ├── Animacion.hpp   ← Componente de animación por spritesheet
│   ├── Input.hpp       ← Teclado + gamepad unificados
│   ├── Particulas.hpp  ← Sistema de partículas
│   ├── Jugador.hpp
│   ├── Gancho.hpp
│   ├── Camara.hpp
│   ├── Mapa.hpp
│   └── HUD.hpp
├── assets/
│   ├── images/
│   │   ├── jugador1.png      ← Spritesheet J1 (768×512, fondo transparente)
│   │   ├── jugador2.png      ← Spritesheet J2 (768×512, fondo transparente)
│   │   ├── bg_layer1.png     ← Fondo parallax — capa lejana
│   │   ├── bg_layer2.png     ← Fondo parallax — capa media
│   │   └── bg_layer3.png     ← Fondo parallax — primer plano
│   ├── music/
│   │   └── musica.ogg
│   └── fonts/
│       └── fuente.ttf
├── bin/                   ← Ejecutable generado (ignorado por git)
├── obj/                   ← Objetos compilados (ignorado por git)
├── gallery/cover.png
├── screenshots/
├── video/demo.mp4
└── makefile
```

---

## 🚀 Compilar y Ejecutar

Abre una terminal **MSYS2 MINGW64** dentro de VSCode y ejecuta:

```bash
# Compilar todo
make all

# Compilar y ejecutar
make run

# Limpiar
make clean
```

> ⚠️ La terminal debe ser **MSYS2 MINGW64**, no PowerShell ni CMD.

---

## 🎮 Descripción del Juego

### 🎯 Objetivo

Todos los jugadores corren por el mismo nivel en circuito. La cámara sigue al que va al frente — si quedas fuera del borde izquierdo, eres **eliminado**. El primero en ganar **3 rondas** gana el juego.

---

### 🕹️ Controles

#### Teclado

| Acción | Jugador 1 | Jugador 2 |
|--------|-----------|-----------|
| Moverse | `A` / `D` | `←` / `→` |
| Saltar | `W` | `↑` |
| Deslizarse | `S` | `↓` |
| Gancho | `LShift` | `RShift` |

#### Gamepad (Xbox / PlayStation)

| Acción | Botón |
|--------|-------|
| Moverse | Stick izquierdo |
| Saltar | `A` / Cruz |
| Deslizarse | `B` / Círculo |
| Gancho | `LB` / `RB` / L1 / R1 |
| Pausa | `Start` / `Options` |

El juego detecta automáticamente si hay gamepad conectado y le da prioridad sobre teclado.

---

### ⚙️ Mecánicas

**🪝 Grappling Hook**
El gancho usa raycast instantáneo (no hay bug de "atravesar paredes"). Se dispara en la dirección que miras, con un ángulo de 30° hacia arriba. Al anclarse, una cuerda elástica te jala progresivamente. Presiona el botón de gancho de nuevo para soltarlo.

**💨 Sistema de velocidad y aceleración**
La aceleración es continua mientras corres. Hay poca fricción en el aire para mantener el momentum. Las rampas tienen fricción casi nula para impulsar al jugador.

**🎥 Cámara dinámica**
La cámara sigue al líder con interpolación suave. El jugador que queda 80px fuera del borde izquierdo es eliminado automáticamente.

**📦 Cajas de items**
Distribuidas por el nivel, flotan y rotan. Al recogerlas otorgan un power-up (en desarrollo).

**✨ Partículas**
- Polvo al correr y aterrizar
- Chispas al anclar el gancho
- Estela de velocidad a alta velocidad
- Confetti al ganar una ronda

---

### 🏆 Características

- Multijugador local 2 jugadores (teclado compartido o gamepad)
- Soporte nativo para Xbox y PlayStation (SFML Joystick API)
- Física con Box2D — gravedad, fricción, joints de cuerda
- Gancho con raycast (sin bugs de tunelado)
- Animaciones por spritesheet: idle, correr, saltar, deslizar
- Fondo con efecto parallax de 3 capas
- Sistema de partículas procedural
- HUD con velocímetro, indicador de gancho y contador de rondas
- Pantalla de victoria animada con pop y confetti
- Sistema de rondas (primero en 3 gana el juego)
- Coyote time en el salto (más justo y fluido)

---

### 👥 Equipo

- **Integrantes**:
  Dafne Jackeline Reynoso Sauceda (@dafners)
  Arturo Maximiliano Salazar Sanchez (@MaxSalazar35)

---

### 🛠️ Tecnologías

| Herramienta | Versión | Uso |
|------------|---------|-----|
| C++ | 17 | Lenguaje principal |
| SFML | 2.x | Gráficos, audio, ventana, joystick |
| Box2D | 2.x | Motor de física 2D |
| MinGW64 / MSYS2 | — | Compilador en Windows |
| Visual Studio Code | — | Editor |
| Make | — | Build system |

---

### 📜 Créditos

- Inspirado en **SpeedRunners** de tinyBuild (mecánicas originales, arte y código propios)
- Música: [mureka.ai](https://mureka.ai)
- Efectos de sonido: [Pixabay](https://pixabay.com/sound-effects/) — licencia libre de derechos
- Fuentes: [dafont.com](https://www.dafont.com/es/)
- Motor de física: [Box2D](https://box2d.org/)
- Gráficos/Audio: [SFML](https://www.sfml-dev.org/)

---

## ⚠️ Errores comunes

**No compila — "undefined reference"**
→ Verifica que la terminal sea MSYS2 MINGW64 y que SFML + Box2D estén instalados con `pacman`.

**Los sprites no aparecen**
→ Coloca los archivos PNG en `assets/images/` con los nombres exactos del README. El juego funciona sin ellos (muestra hitboxes de color) pero sin sprites.

**El gamepad no responde**
→ Conecta el control **antes** de iniciar el juego. SFML detecta joysticks al inicio.

**La música no suena**
→ Coloca un archivo `.ogg` en `assets/music/musica.ogg`. MP3 no es soportado por SFML. Los efectos de sonido van en `assets/sounds/`.

---

## 🔄 Flujo de trabajo (CETUS)

```bash
git add .
git commit -m "feat: descripción del cambio"
git push origin main
# → GitHub Action publica automáticamente en CETUS
```
