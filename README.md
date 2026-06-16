# raylibgl-car-hw

OpenGL **Model & Viewer** exercise (Shenkar, Applied Computer Graphics, Ex3),
implemented in **C++ with [raylib](https://www.raylib.com/) + rlgl**.

The goal is a hierarchical **car** model (a Scooby-Doo "Mystery Machine" van)
with an interactive viewer: trackball rotation, perspective zoom, lighting, and
the standard wireframe / axes / light-marker toggles. The original assignment is
fixed-function OpenGL (Java/JOGL); rlgl's immediate mode (`rlBegin` / `rlVertex3f`
/ `rlPushMatrix`) is the direct translation of that API.

## Build & run

Requires **CMake 3.15+**, **clang** (forced in `CMakeLists.txt`), and **Ninja**.
raylib 6.0 is fetched automatically via CMake `FetchContent`.

```sh
cmake -S . -B build -G Ninja
cmake --build build
./bin/raylibgl            # Windows: bin\raylibgl.exe
```

> **Note:** the exit key is **F10** (not Esc). The running app locks the
> executable, so **close it before rebuilding** or the link step fails with
> "permission denied".

New source files under `src/` are picked up automatically (CMake
`GLOB_RECURSE CONFIGURE_DEPENDS`) — no need to edit `CMakeLists.txt`.

## Controls

| Input | Action |
|---|---|
| Left-drag | Rotate the model (virtual trackball) |
| Mouse wheel | Zoom |
| `P` | Toggle wireframe / filled |
| `A` | Toggle XYZ axes |
| `O` | Toggle orthographic / perspective (dev aid, not required) |
| `F10` | Quit |
| `L` | *(planned)* toggle light-source markers |

## Project layout

```
src/
  main.cpp                 entry point; window size + title
  app/
    Application.{hpp,cpp}  window, main loop, input toggles, scene draw
    TrackballCamera.{hpp,cpp}  Camera3D (view + zoom + proj) + trackball rotation
    DebugControls.{hpp,cpp}    (inherited, currently disabled/commented)
  model/
    Primitives.{hpp,cpp}   small draw helpers: drawAxes, drawBox, drawCylinder
    Scooby-van.{hpp,cpp}   the van model: drawChassis / drawWheel / drawLight / drawCar
car-parts.md               traced part positions/sizes for the van (build reference)
```

### How it works
- **Viewing:** `TrackballCamera` keeps a fixed `Camera3D` (eye looking at the
  origin) for the view + wheel zoom, and an accumulated rotation `Matrix` from
  the trackball (assignment Appendix A). The rotation is applied to the geometry
  via `rlMultMatrixf` inside `BeginMode3D` — i.e. the **world rotates about the
  origin in the ModelView**, the camera does not move.
- **Drawing:** `Application::drawSceneRlgl()` calls `model::drawCar()`, which
  composes the van from `drawChassis` / `drawWheel` (+ inline roof-rack, bumper
  and door parts) using the rlgl matrix stack (`rlPushMatrix` / `rlTranslatef`).
  `drawCar` re-centers the model's visual center (0, 1.24, 0) onto the origin so
  the trackball rotates about the middle of the van. Parts that map to a box or
  cylinder use `model::drawBox` / `model::drawCylinder` (which pick the filled or
  `...Wires` variant from the wireframe toggle); the tapered body and the door
  use a local `drawExtrudedX` helper — a Y-Z silhouette extruded along X, with
  ear-clipping triangulated end-caps so concave profiles (e.g. the door) render
  correctly. Coordinates throughout match `car-parts.md` (Y up, front = −Z).
- **Culling:** back-face culling is on (`rlEnableBackfaceCulling`, CCW = front).

## Status

**Done:** resizable window, virtual trackball, perspective + zoom, back-face
culling, axes toggle (`A`), wireframe toggle (`P`), primitive wrappers.

**Van model:** the placeholder car is replaced by the real Mystery Machine van
in `model/Scooby-van.{hpp,cpp}`, built incrementally from `car-parts.md` and
composed in `drawCar` via the rlgl matrix stack (each part modeled once at a
local origin and mirrored with `rlTranslatef`). Required features:
- **Chassis** — done: one turquoise tapered solid (hexagonal Y-Z side profile
  extruded along X via `drawExtrudedX`), not two boxes. Filled polygons.
- **Doors** — done: a darker-turquoise filled *trapezoid* panel per side (5-vertex
  profile) **plus the required door-seam outline drawn as wireframe lines, always
  (independent of `P`)** — `drawProfileOutline`.
- **Windows** — **TODO (required):** filled polygons (windshield + side windows).
  The only required modeling element still missing. (Intended approach: flat
  filled quads laid on the body faces, nudged slightly proud to avoid z-fighting
  — prototyped once, then reverted.)
- **Tires ×4** — done: dark cylinders bounded by disks, axle along X, at the four
  corners.
- **Front lights ×2** — done as geometry: yellow cylinders bounded by disks on the
  −Z face (no light source yet — see lighting below).

Embellishments (allowed extras): metal-grey roof-rack slats, front/back bumpers,
yellow rear tail-lights, a front-mounted spare tire, and side mirrors.

**Still required (left to the lighting pass — being handled separately):**
1. Lighting — ≥2 positional lights, demonstrate diffuse + specular, set
   materials, hand-set normals (enable `GL_NORMALIZE`-equivalent). Custom geometry
   here (`drawExtrudedX`, the door outline) currently emits **no `rlNormal3f`** —
   normals must be added for correct shading.
2. Light-source marker spheres + `L` toggle (emissive only).

### Known caveats
- **Built-in cylinders have no normals** ([raylib #4808](https://github.com/raysan5/raylib/issues/4808)),
  so `DrawCylinderEx` wheels/lights won't light correctly. At the lighting step,
  switch the lit cylinders to a mesh (`GenMeshCylinder`), restore a handmade
  cylinder, or compute normals in-shader.
- **`glPolygonMode` wireframe** (`rlEnableWireMode`) rendered nothing on this
  setup, so wireframe uses the `...Wires` line variants instead (reliable, and
  portable to GLES/web).
- **Doors are wireframe lines** (a modeling requirement) — the seam outline is
  drawn as lines always, independent of the `P` toggle, via `drawProfileOutline`.

## Reference model

`car-parts.md` holds positions/sizes traced loosely from a low-poly Mystery
Machine glTF. The glTF is used **only as a visual reference** — no external mesh
files are loaded (the assignment forbids it).
