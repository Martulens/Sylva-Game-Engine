# Sylva

A small real-time rendering engine written in modern C++ and OpenGL 3.3 core.
Sylva generates a procedural forest, populates it with instanced vegetation,
and renders an animated character inside a day/night-cycled scene.

Created as a project for school course. Now it is under reconstruction and awaiting further rework.

<!-- Add a screenshot at docs/screenshot.png and uncomment:
![Sylva in-game](docs/screenshot.png)
-->

## Features

- **Procedural terrain** — height map built from multi-octave Perlin noise,
  resampled per-vertex on a configurable grid.
- **Chunk-based streaming** — trees and decorative details (bushes, stones,
  flowers, mushrooms, grass) are generated on demand around the player, driven
  by a grayscale tree-density map.
- **Hardware instanced rendering** — each mesh type maintains a per-instance
  matrix VBO and is drawn with `glDrawElementsInstanced`, so hundreds of trees
  cost one draw call per species.
- **Multi-layer PBR-style terrain** — four material layers (grass, sand, water,
  tiles), each with base colour, normal, roughness, height and emissive maps,
  blended through an RGB blend map sampled per fragment.
- **Animated water** — UV scroll plus sinusoidal ripple applied to the "water"
  blend layer in the terrain shader.
- **Skeletal animation with GPU skinning** — FBX rigs loaded via Assimp, bone
  offsets uploaded as a `mat4[256]` uniform, per-vertex 4-bone linear blend
  skinning in the vertex shader. Animation key-frames sampled on the CPU per
  frame from the full-rig take.
- **Rigid sub-mesh binding** — sub-meshes that ship with zero skin weights
  (head, hair, eyes, mouth on the bundled character) are bound as synthetic
  bones attached to their host node in the skeleton so they follow the
  skeleton's animation.
- **Day/night cycle** — sun and moon orbit the scene, driving directional
  light direction/colour, fog density/gradient and sky tint. Sun acts as a
  directional light, moon as a point light.
- **Dynamic lights** — up to 20 placeable point-light torches plus a player
  flashlight implemented as a cut-off spotlight.
- **Cube-mapped skybox** — six-face cubemap with blend factor driven by sun
  elevation.
- **Four camera modes** — first-person, third-person orbit, fixed top-down,
  and an animated Catmull-Rom spline camera orbiting the player. Transitions
  are smooth-stepped between modes.
- **Scene interaction** — click a tree to replace it with a random species,
  press `T` to place a torch at the crosshair's terrain intersection, `C` to
  play a wave animation, `B` to switch between the bundled character models.

## Build

Sylva uses CMake with `FetchContent` to pull its dependencies, so a clean
checkout builds with no manual library setup:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/sylva
```

Tested on Linux (GCC 15), macOS (Clang), and Windows (MSVC) via the GitHub
Actions matrix in `.github/workflows/ci.yml`.

### Requirements

- CMake 3.20+
- A C++17 compiler
- OpenGL 3.3-capable GPU
- Python 3 with `jinja2` (used at configure time by the glad generator)
- On Linux: X11 development headers (`xorg-dev`, `libxkbcommon-dev`,
  `libgl1-mesa-dev` on Debian/Ubuntu)

## Controls

| Key(s)                 | Action                                             |
|------------------------|----------------------------------------------------|
| `W A S D` / Arrow keys | Move                                               |
| Mouse                  | Look around                                        |
| `V`                    | Cycle camera (first/third person, top-down, spline)|
| `C`                    | Toggle wave animation                              |
| `T`                    | Place torch at crosshair                           |
| `B`                    | Switch character model                             |
| `L`                    | Toggle flashlight                                  |
| `Tab`                  | Toggle day/night progression                       |
| `R`                    | Restart scene                                      |
| `M`                    | Toggle cursor capture                              |
| `O`                    | Reset third-person pitch                           |
| `P`                    | Reset third-person distance                        |
| Mouse click            | Replace clicked tree with a random one             |
| `Esc`                  | Quit                                               |

## Tech stack

| Area                | Library                            |
|---------------------|------------------------------------|
| Windowing / input   | GLFW 3.4                           |
| GL function loader  | glad 2.0.6 (GL 3.3 core)           |
| Math                | GLM 1.0.1                          |
| Model loading       | Assimp 5.0.1 (FBX, OBJ, Collada, glTF) |
| Texture loading     | stb_image                          |
| Build               | CMake 3.20+ with FetchContent      |
| CI                  | GitHub Actions, Linux + macOS + Windows |

## Project layout

```
src/
  main.cpp              application entry, GLFW window, input, main loop
  camera.{h,cpp}        view/projection + four camera modes
  gamestate.{h,cpp}     global mutable state (time of day, keys, camera transition…)
  terrain.{h,cpp}       procedural terrain generation and rendering
  meshgeometry.{h,cpp}  FBX loading, skinning prep, bone hierarchy
  modeltexture.{h,cpp}  PBR material loading
  objects.{h,cpp}       Object/Skybox/Light/Player/InstanceGroup scene classes
  render.{h,cpp}        uniform setup, draw routines for each object type
  shaders.{h,cpp}       shader program wrapper with cached uniform locations
  noise.{h,cpp}         2D Perlin noise
  sylva/sylva.{h,cpp}   platform layer — texture/shader helpers, time
  shaders/*.vert|.frag  GLSL sources
  models/               FBX assets (character + trees + details)
  textures/             PNGs for terrain layers, character, skybox, maps
```

## License

Released under the MIT License.
