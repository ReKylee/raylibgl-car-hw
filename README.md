# raylibgl-car-hw

Ex3 "Model & Viewer" for Shenkar's Applied Computer Graphics course, written in
C++ with [raylib](https://www.raylib.com/) and rlgl. The model is a Scooby-Doo
"Mystery Machine" van; the viewer gives you trackball rotation, perspective zoom,
lighting, and the wireframe / axes / light-marker toggles the assignment asks for.

The original exercise targets fixed-function OpenGL (Java/JOGL). rlgl's immediate
mode (`rlBegin` / `rlVertex3f` / `rlPushMatrix`) maps onto that API almost
one-to-one, so the same modeling and matrix-stack techniques apply.

## Build and run

You need CMake 3.15+, clang (forced in `CMakeLists.txt`), and Ninja. raylib 6.0
and raygui are fetched automatically by `FetchContent`.

```sh
cmake -S . -B build -G Ninja
cmake --build build
bin/raylibgl.exe        # on Linux/macOS: ./bin/raylibgl
```

The exit key is F10, not Esc. A running instance holds a lock on the executable,
so close it before rebuilding or the link step fails with "permission denied".
Source files under `src/` are globbed with `CONFIGURE_DEPENDS`, so new files are
picked up without touching `CMakeLists.txt`. The `assets/` folder is copied next
to the executable as part of the build.

## Controls

| Input | Action |
|---|---|
| Left-drag | Rotate the model (virtual trackball) |
| Mouse wheel | Zoom (moves the camera, perspective only) |
| `P` | Wireframe / filled polygons |
| `A` | XYZ axes on/off |
| `L` | Light-source marker spheres on/off |
| `O` | Orthographic / perspective (design aid) |
| `G` | Floor grid (design aid) |
| `X` | Axes rotate with the model |
| `R` | Reset rotation |
| `F10` | Quit |

The same toggles are available in a collapsible raygui debug panel, drawn over
the 3D view by `DebugControls`.

## Project layout

```
src/
  main.cpp                   entry point; window size + title
  app/
    Application.{hpp,cpp}     window, main loop, lighting shader, draw passes
    TrackballCamera.{hpp,cpp} Camera3D (view + zoom + projection) + trackball rotation
    DebugControls.{hpp,cpp}   raygui overlay panel + key handling
  model/
    Primitives.{hpp,cpp}      drawAxes, drawBox, drawCylinder (hand-set normals)
    Scooby-van.{hpp,cpp}      the van; Scooby-van.cpp is generated (see below)
  reference/                  the source glTF's embedded texture, kept for reference
assets/
  MysteryMachineSideDecal.png the flame side stripe
car-parts.md                  early hand-traced blockout (historical reference)
```

The generator lives one level up, in `scooby-do-the-mystery-machine_zip/`
(`gen2.py` + `template2.cpp`).

## How it works

### Viewing

`TrackballCamera` keeps a fixed `Camera3D` looking at the origin and accumulates a
rotation `Matrix` from the trackball (Appendix A: project the mouse onto a sphere
before and after a drag, build the rotation between the two vectors, compose it
onto the stored matrix). `drawSceneRlgl` applies that matrix with `rlMultMatrixf`
inside `BeginMode3D`, so the world rotates about the origin in the ModelView and
the camera itself never moves. Zoom moves the camera along its view direction.
The window is resizable; the projection uses the live aspect ratio so the image
never distorts.

### Modeling

The car is composed in `drawCar` from one helper per feature, each wrapping its
geometry in `rlPushMatrix` / `rlPopMatrix` so it works in its own local frame.
Repeated parts are modeled once and placed with translations:

- The whole body is a single extruded-hull mesh (`drawBodyShell`): a side
  silhouette in the Y-Z plane, swept across the width, with a half-hexagon
  wheel-arch notch at each end. Front and back come out near-symmetric, so the
  left and right walls are the same outline emitted at `+x` and `-x`.
- Four wheels share one `drawWheel`, placed at the four axle positions. A wheel is
  a `drawCylinder` tyre (a cylinder capped by two disks) with a badge on each face.
- The front emblem and the wheel-face badges are the same "yellow disk plus three
  crossing sticks" shape, modeled once in `drawEmblem` and oriented by the caller.
- The windshield and both front doors are framed inset windows: a body-colour
  frame, reveal walls stepping inward, and a recessed semi-transparent glass quad.
- Head and tail lights, side mirrors, the bench seat, grey bumpers, roof bars, and
  the roof aquarium with its goldfish round out the model.

Symmetry is done with rotations and translations rather than a negative scale, so
front faces stay wound the same way and no `rlSetFrontFace` flip is needed.

### Lighting

raylib 6's default shader does not do the per-light work the assignment wants, so
`Application` loads a small GLSL shader (`LIGHTING_VERTEX_SHADER` /
`LIGHTING_FRAGMENT_SHADER`). It runs two positional point lights (one warm, one
cool), each contributing Lambert diffuse and Blinn-Phong specular, plus a soft
hemispheric ambient term and a thin fresnel rim. Two cone spotlights ride along
with the van at the headlights and throw a warm pool onto the ground in front.

The head and tail light cores reuse the same shader with an emission term turned
up, so they output their own colour instead of being shaded; an additive bloom
halo is drawn over them afterwards. Marker spheres sit at the two point-light
positions and are emissive only (unaffected by the lights); `L` hides them.

Normals are set by hand everywhere the geometry is emitted (`rlNormal3f` in the
hull sweep, the cylinder walls, the quads, the sphere bands). Because the model is
drawn in immediate mode under `rlPushMatrix`, rlgl bakes the trackball transform
straight into the vertices, so the shader reads `vertexPosition` / `vertexNormal`
directly rather than re-applying `matModel`.

Back-face culling is enabled (CCW front). It is switched off in two narrow places:
the body hull, so the cabin interior is visible through the glass, and the
double-sided window glass.

## The generated model file

`model/Scooby-van.cpp` is generated, not written by hand. `gen2.py` reads the
Blockbench glTF in `scooby-do-the-mystery-machine_zip/source/`, throws away the
~127 body cuboids (the hull mesh replaces them), keeps the two roof bars as a baked
data table, and substitutes that table into `template2.cpp` (which holds all the
actual drawing code). To change the model, edit `template2.cpp` and re-run:

```sh
cd ../scooby-do-the-mystery-machine_zip
python gen2.py
```

Editing `Scooby-van.cpp` directly is pointless because the next generator run
overwrites it.

## Mapping to the assignment

| Requirement | Where |
|---|---|
| Polygons (filled) | hull, doors, windows, bumpers, seats — triangles and quads |
| Cylinder bounded by disks | `drawCylinder` (tyres, emblem disks, fish eyes) |
| Recursive matrix-stack structure | one helper per feature, each under `rlPushMatrix` |
| Model each part once, mirror | wheels x4, lights x2, doors, decal, badges |
| Trackball rotation | `TrackballCamera` (Appendix A) |
| Perspective + zoom | `Camera3D`, wheel moves the camera |
| Resizable, non-distorting window | `FLAG_WINDOW_RESIZABLE`, aspect-correct projection |
| `P` / `A` / `L` toggles | `DebugControls` |
| >= 2 positional lights, diffuse + specular | lighting shader, two point lights |
| Light-marker spheres (emissive), `L` to hide | `drawLightMarkers` |
| Hand-set normals | every emitted primitive |
| Back-face culling | on by default (CCW front) |

Beyond the minimum, the model adds spotlights, emissive light cores with bloom,
the framed inset doors and windows, the textured side decal, mirrors, a bench
seat, and the roof aquarium.
