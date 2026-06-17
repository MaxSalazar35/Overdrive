# Overdrive

Juego de plataformas multijugador de velocidad para 2 jugadores, inspirado en SpeedRunners. Desarrollado en **C++ con SFML y Box2D**, compilado con **MinGW64 (MSYS2)** en Visual Studio Code.

¿Quién es más rápido? Corre, salta y deja atrás a tu rival antes de que la pantalla te deje fuera. Gana altura y usa el gancho para volar sobre las plataformas, pero ten cuidado, las rampas son resbaladizas y los picos no perdonan. Cada jugador tiene 3 vidas, úsalas bien. OVERDRIVE es un juego de plataformas frenético para 2 jugadores en un mapa infinito donde cada segundo cuenta y perder el ritmo significa perder la partida.



---

## 🛠️ Prerequisitos

- [Herramientas (VSCode, MSYS2, Git)](./docs/herramientas.md)
- [Extensiones de VSCode](./docs/extensiones.md)
- [Librerías SFML + Box2D](./docs/librerias.md)
- [Clonar el repositorio](./docs/fork.md)

---

## 🚀 Compilar y Ejecutar

Abre una terminal **MSYS2 MINGW64** y ejecuta:

```bash
# Compilar todo
make all

# Compilar y ejecutar
make run

# Limpiar
make clean

````
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

---

### ⚙️ Mecánicas

**🪝 Grappling Hook**
El gancho usa raycast instantáneo (no hay bug de "atravesar paredes"). Se dispara en la dirección que miras, con un ángulo de 30° hacia arriba. Al anclarse, una cuerda elástica te jala progresivamente. Presiona el botón de gancho de nuevo para soltarlo. Solamente se activa cuando el jugador alcanza una altitud requerida.

**💨 Sistema de velocidad y aceleración**
La aceleración es continua mientras corres. Hay poca fricción en el aire para mantener el momentum. Las rampas resbalan al jugador.

**🎥 Cámara dinámica**
La cámara sigue al líder con interpolación suave. El jugador que queda 80px fuera del borde izquierdo es eliminado automáticamente.

**📦 Cajas**
Distribuidas por el nivel, flotan y rotan. Al recogerlas te hacen frenar.

**✨ Partículas**
- Polvo al correr y aterrizar
- Chispas al anclar el gancho
- Estela de velocidad a alta velocidad
- Confetti al ganar una ronda

---

### 🏆 Características

- Multijugador local 2 jugadores (teclado compartido)
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

**El gancho no funciona**
→ Asegúrate de estar lo más alto que puedas para usar el gancho, si lo presionas desde el suelo no funcionará.

---