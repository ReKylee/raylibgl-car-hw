# raylibgl-car-hw

By: Kylee Benisty and Peleg Ben Dor

Ex3 "Model & Viewer" for Shenkar's Applied Computer Graphics course - Scooby-Doo "Mystery Machine" van modeled in C++ with [raylib](https://www.raylib.com/) and rlgl.

## Dependencies

- **CMake** 3.15 or newer
- **Clang** (`clang` / `clang++`) - forced in `CMakeLists.txt`
- **Ninja** build system
- **Git** (raygui is fetched from a git repository)

raylib 6.0 and raygui are downloaded automatically by CMake's `FetchContent`, so you do not need to install them yourself. On Linux you also need the usual OpenGL / X11 development headers that raylib builds against.

## Compiling

### Unix (Linux / macOS)

```sh
cmake -S . -B build -G Ninja
cmake --build build
./bin/raylibgl
```

On Debian/Ubuntu, install the toolchain and raylib's system dependencies first:

```sh
sudo apt install cmake clang ninja-build git \
    libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libxi-dev
```

### Windows

```sh
cmake -S . -B build -G Ninja
cmake --build build
bin\raylibgl.exe
```

Make sure `clang`, `cmake`, `ninja`, and `git` are on your `PATH` (the LLVM toolchain plus the standalone Ninja release work well).

The `assets/` folder is copied next to the executable as part of the build.

## Controls

| Input | Action |
|---|---|
| Left-drag | Rotate the model (virtual trackball) |
| Mouse wheel | Zoom (moves the camera, perspective only) |
| `P` | Wireframe / filled polygons |
| `A` | XYZ axes on/off |
| `L` | Light-source marker spheres on/off |
| `O` | Orthographic / perspective |
| `G` | Floor grid |
| `X` | Axes rotate with the model |
| `R` | Reset rotation |
| `F10` | Quit |

The same toggles are also available in a collapsible raygui debug panel drawn
over the 3D view.
