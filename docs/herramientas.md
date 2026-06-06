# Herramientas necesarias

## Visual Studio Code
Descargar desde [code.visualstudio.com](https://code.visualstudio.com/).

## MSYS2 (Windows)
Descargar desde [msys2.org](https://www.msys2.org/).

Agregar al PATH y reiniciar:
- `C:\msys64\mingw64\bin`
- `C:\msys64\usr\bin`

## Git
Descargar desde [git-scm.com](https://git-scm.com/). Instalar con opciones por defecto.

## Configurar terminal MSYS2 en VSCode
En `settings.json` de VSCode:

```json
"terminal.integrated.profiles.windows": {
    "MSYS2 MINGW64": {
        "path": "C:\\msys64\\usr\\bin\\bash.exe",
        "args": ["--login", "-i"],
        "env": { "MSYSTEM": "MINGW64" }
    }
},
"terminal.integrated.defaultProfile.windows": "MSYS2 MINGW64"
```
