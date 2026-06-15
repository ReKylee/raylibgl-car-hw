# Car parts list — build reference

Traced loosely from the Mystery Machine glTF
(`scooby-do-the-mystery-machine_zip/source/scooby_do.gltf`) for the Ex3 OpenGL
Model & Viewer assignment. Numbers are a **simplified blockout** to build
against by hand — not exact transcriptions.

## Conventions

- **Y up**, the van **faces −Z**.
- Model sits on the ground at **y ≈ 0** (wheel bottoms = lowest point).
- Visual center ≈ **(0, 1.24, 0)** → use as the **trackball rotation pivot**.
- Overall size ≈ **2.6 (W) × 2.65 (H) × 4.2 (L)**, units as in the glTF.

## Required feature tree (from the PDF)

- **Chassis** — filled polygons
  - **Doors** — wireframe lines
  - **Windows** — filled polygons
- **Tires** ×4 — cylinder bounded by disks
- **Front lights** ×2 — cylinder bounded by disks
- *(scene, not car geometry)* 2 positional lights + emissive marker spheres

## Parts (model units)

| Feature | Primitive | Center (x, y, z) | Dims | Build / symmetry |
|---|---|---|---|---|
| Chassis – lower body | filled polys (box) | (0, 0.90, 0.20) | 2.0 × 1.1 × 3.9 | model half (x≥0), mirror `scale(-1,1,1)` |
| Chassis – roof/cabin | filled polys (box) | (0, 1.90, 0.40) | 1.9 × 1.0 × 3.2 | sits on lower body |
| Windows | filled polys | windshield (0, 1.5, −1.55); sides (±1.0, 1.5, 0.3) | ~1.6×0.7 / ~2.0×0.7 | front + L/R (mirror) |
| Doors | wireframe lines | (±1.0, 1.0, 0.3) | outline on body side | rectangle outline, mirror |
| Tire ×4 | cylinder + 2 disks | front (±0.80, 0.33, −0.90); rear (±0.80, 0.33, +1.00) | r ≈ 0.41, width ≈ 0.37 (axis = X) | model one, place ×4 |
| Front light ×2 | cylinder + 2 disks | (±0.40, 1.05, −1.85) | r ≈ 0.17, depth ≈ 0.12 (axis = Z) | model one, mirror in X |

### Scene (not car geometry)

| Item | Type | Placement |
|---|---|---|
| Light sources ×2 | positional | your choice, e.g. (±2, 3.5, −2) above/front of model |
| Marker spheres | emissive only | at each light's position; toggle with `l` |

## Notes

- **Symmetry** is exact in the source: wheels at x = ±0.80, lights at x = ±0.40,
  body left/right symmetric. Model each piece once, mirror with negative scaling
  (then fix winding via `glFrontFace` / `rlSetFrontFace`).
- The source's two primitive types line up with the two you must build:
  `octagon` meshes = wheels + headlights (cylinder+disk); `cube` meshes = body.
- Front = −Z: headlights and the windshield are on the −Z face; front wheels are
  the −Z pair, rear wheels the +Z pair.

## Raw measured anchors (for reference)

- Model world bbox: min (−1.305, −0.087, −2.022), max (1.305, 2.56, 2.20)
- Wheel clusters (octagons, y ≈ 0.33): x ≈ ±0.80; front z ≈ −0.9, rear z ≈ +1.0;
  diameter ≈ 0.82 → r ≈ 0.41; width (X) ≈ 0.37
- Headlight clusters (octagons, y ≈ 1.05, z ≈ −1.85): x ≈ ±0.40; r ≈ 0.17
- Body cubes span: x [−1.18, 1.18], y [0.37, 2.50], z [−1.82, 2.20]
