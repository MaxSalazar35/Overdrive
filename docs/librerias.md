# Librerías (MSYS2 MINGW64)

Abrir la terminal **MSYS2 MINGW64** y ejecutar:

## 1. DevTools
```bash
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
```
Presionar Enter para instalar todos.

## 2. SFML
```bash
pacman -S mingw-w64-x86_64-sfml
```

## 3. Box2D
```bash
pacman -S mingw-w64-x86_64-box2d
```

## Verificar
```bash
g++ --version
```
Debe mostrar `g++ (Rev..., Built by MSYS2 project)`.
