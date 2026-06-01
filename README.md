# Overdrive

# PlantillaProyecto

Este es el template oficial para los proyectos de la materia Proyecto 252. Sigue las instrucciones para configurar tu proyecto y sincronizarlo automáticamente con la galería de CETUS.

## 📋 Estructura Requerida 

Tu repositorio debe seguir esta estructura exacta:

```
proyecto-252/
├── .github/
│   └── workflows/
│       └── publish.yml          ← GitHub Action (ya incluido)
│
├── video/
│   └── demo.mp4                 ← Video de gameplay (max 2 min)
│
├── gallery/
│   └── cover.png                ← Imagen de portada (720x1080)
│
├── screenshots/
│   ├── screenshot1.png          ← Capturas del juego (mínimo 3)
│   ├── screenshot2.png
│   └── screenshot3.png
│
├── bin/
│   └── JuegoProyecto.exe        ← Ejecutable del juego
│
├── assets/
│   ├── textures/
│   ├── sounds/
│   └── ...                      ← Todos los recursos necesarios
│
├── README.md                    ← Este archivo (edítalo con tu proyecto)
└── .gitignore
```

## 🚀 Cómo Usar Este Template

### 1. Haz Fork de Este Repositorio

Copia los archivos que te hagan falta o haz fork de este repositorio a tu cuenta de GitHub.

### 2. Configura en CETUS

1. Ve a [CETUS](https://cetus.logiasimbolica.com)
2. Inicia sesión con tu cuenta
3. Ve a **Proyectos** y entra a tu proyecto
4. En la sección "Repositorio de GitHub", haz clic en **Configurar**
5. Pega la URL de tu repositorio (por ejemplo: `https://github.com/tu-usuario/mijuego`)
6. Guarda la configuración

### 3. Organiza Tus Archivos

Coloca tus archivos en las carpetas correspondientes siguiendo la estructura de arriba.

**Requisitos importantes:**

#### Video (`video/demo.mp4`)

- Duración máxima: **2 minutos**
- Formato: MP4
- Contenido: Gameplay mostrando las características principales de tu juego
- Resolución recomendada: 1280x720 o superior

#### Portada (`gallery/cover.png`)

- Dimensiones: **720x1080 pixels** (vertical)
- Formato: PNG
- Representa visualmente tu juego (puede ser logo + screenshot)

#### Screenshots (`screenshots/*.png`)

- Mínimo: **3 capturas**
- Dimensiones: **1080x720 pixels** cada una
- Formato: PNG
- Muestra diferentes aspectos del juego

#### Ejecutable (`bin/*.exe`)

- Incluye el `.exe` principal de tu juego
- Si necesitas DLLs adicionales, inclúyelas también

#### Assets (`assets/`)

- Todos los recursos que tu juego necesita para funcionar
- Texturas, sonidos, música, modelos 3D, etc.
- Organiza en subcarpetas

#### README.md

- Edita este archivo con la descripción de tu proyecto
- Incluye: objetivo, controles, mecánicas, créditos

### 4. Publica Tu Proyecto

Cada vez que hagas **push a la rama `main`**, el GitHub Action se ejecutará automáticamente y:

1. ✅ Empaquetará tu ejecutable y assets en un ZIP
2. ✅ Creará un Release con versión automática
3. ✅ Notificará a CETUS para actualizar la galería
4. ✅ Tu proyecto aparecerá en https://cetus.logiasimbolica.com/galeria

**No necesitas hacer nada más** - la sincronización es automática.

## 🔄 Flujo de Trabajo

```
1. Desarrollas tu juego localmente
2. Actualizas archivos en las carpetas correspondientes
3. git add . && git commit -m "Actualización X"
4. git push origin main
5. GitHub Action se ejecuta automáticamente
6. ✨ Tu proyecto se actualiza en la galería
```

## ⚠️ Validaciones

El sistema validará automáticamente:

- ✓ Repositorio configurado en CETUS
- ✓ Estructura de carpetas correcta
- ✓ Video existe y es accesible
- ✓ Portada existe (cover.png)
- ✓ Mínimo 3 screenshots
- ✓ Ejecutable existe
- ✓ README.md presente

Si algo falla, recibirás un error en el log del GitHub Action.

## 📊 Tracking de Progreso

Cada commit que hagas se registrará en CETUS para evaluar:

- Frecuencia de commits
- Progreso a lo largo del tiempo
- Contribuciones de cada integrante

Asegúrate de hacer commits significativos con mensajes descriptivos.

## 🆘 Problemas Comunes

### El Action falla

- Verifica que todas las carpetas existan
- Revisa que los archivos tengan los nombres exactos
- Asegúrate de que el repo sea público

### No aparece en la galería

- Confirma que registraste la URL en CETUS
- Verifica que el push fue a la rama `main`
- Revisa los logs del Action en GitHub

### El ejecutable no se descarga

- Asegúrate de que el Release se creó exitosamente
- Verifica que el ZIP contiene bin/ y assets/

## 📝 Descripción del Proyecto

**[Edita esta sección con la información de tu juego]**

### 🎯 Objetivo del Juego

Describe aquí el objetivo principal de tu juego.

### 🎮 Controles

Lista los controles:

- W/A/S/D: Movimiento
- Space: Saltar
- Mouse: Apuntar/Disparar
- etc.

### ⚙️ Mecánicas

Explica las mecánicas principales de tu juego.

### 🏆 Características

- Feature 1
- Feature 2
- Feature 3

### 👥 Equipo

- **Líder**: Nombre Completo (@usuario-github)
- **Integrante 2**: Nombre Completo (@usuario-github)
- **Integrante 3**: Nombre Completo (@usuario-github)

### 🛠️ Tecnologías

- Motor/Framework: Unity/Godot/Unreal/etc.
- Lenguaje: C#/C++/Python/etc.
- Librerías adicionales: ...

### 📜 Créditos

- Assets de terceros utilizados
- Referencias o inspiraciones
- Agradecimientos

---