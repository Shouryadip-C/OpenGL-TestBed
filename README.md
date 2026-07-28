# OpenGL TestBed

A modular C++ rendering testbed for experimenting with OpenGL rendering techniques and CUDA/OpenGL GPU interop, built on a small internal framework (`Test`/`TestMenu`) that lets each demo run in isolation without touching the others.

Each "test" is a self-contained module (own setup, update, render, and ImGui panel) that gets registered with a menu at startup, so new experiments can be added without modifying existing ones.

## Features

- **Test-based architecture** — a `Test` base class (mouse/keyboard input hooks, `onUpdate`, `onRender`, `onImGuiRender`) with a `TestMenu` that registers and switches between demos at runtime.
- **Core OpenGL wrappers** — RAII-style abstractions over `VertexArray`, `VertexBuffer`, `IndexBuffer`, `Shader`, `Texture`, and a `Renderer` for draw/clear calls.
- **Model loading** — `Model`/`Mesh` classes built on Assimp, with the sample `backpack` model (diffuse/normal/roughness/specular/AO maps) included under `res/models`.
- **Camera system** — a free-look/movable 3D camera (`Camera.h/.cpp`) used in the camera and model-loading demos.
- **CUDA/OpenGL interop demo** — a `TestCuda` module running live GPU simulations (Conway's Game of Life and a Forest Fire cellular automaton) with output written directly into an OpenGL-mapped pixel buffer (see below).
- **ImGui tooling** — docking + multi-viewport ImGui integration for per-test controls, an FPS/VSync panel, and a demo window toggle.
- **Screenshot capture** — `ScreenshotManager` captures the window to an image on `F12`.
- **Included demos** — Clear Color, 2D Transformations, 3D Rotating Cubes, Movable 3D Camera, Model Loading, and the CUDA Simulation test.

## CUDA/OpenGL Interoperability

The CUDA demo (`src/cuda_utils/`, wired up in `tests/TestCuda.cpp`) shows a zero-copy path from a CUDA kernel to the screen:

1. An OpenGL **Pixel Unpack Buffer (PBO)** is created and registered with CUDA via `cudaGraphicsGLRegisterBuffer`.
2. Each frame, the PBO is **mapped** (`cudaGraphicsMapResources`) to get a device pointer (`cudaGraphicsResourceGetMappedPointer`).
3. A CUDA kernel writes pixel data directly into that device pointer — currently either:
   - `runLifeSimulationStep` — Conway's Game of Life on a grid, rendered with alive/about-to-die/background coloring, or
   - `runForestSimulationStep` — a forest-fire cellular automaton (empty/tree/burning cells) using `curand` for randomized spread.
4. The PBO is **unmapped** so OpenGL can use it, then uploaded into a texture with `glTexSubImage2D` and drawn to a fullscreen quad (`res/shader/fullscreen_quad.glsl`).
5. Cleanup (`cleanupLifeSimulation` / `cleanupForestSimulation`, `cudaGraphicsUnregisterResource`) runs on simulation switch and on test teardown.

All CUDA error checking goes through a `CUDA_CHECK` macro (`src/cuda_utils/CudaInterop.h`).

## Repository Structure

```
OpenGL-TestBed/
├── CMakeLists.txt              # Top-level build config (project: OpenGL_TestBed, CXX/C/CUDA)
├── src/
│   ├── main.cpp                 # Window/GL/ImGui setup, main loop, test registration
│   ├── Core.h / Core.cpp        # GL_CALL error-checking macro and helpers
│   ├── Renderer.h / .cpp        # Draw/clear wrapper
│   ├── VertexArray, VertexBuffer, IndexBuffer, Shader, Texture   # Core GL object wrappers
│   ├── Camera.h / .cpp          # Movable 3D camera
│   ├── Mesh.h / .cpp, Model.h / .cpp   # Assimp-based model loading
│   ├── Clock.h / .cpp           # Frame timing / FPS limiting
│   ├── ScreenshotManager.h / .cpp
│   └── cuda_utils/
│       ├── CudaInterop.h        # CUDA_CHECK macro, CUDA/GL interop includes
│       ├── Simulations.h / .cu  # Game of Life + Forest Fire kernels
│       └── CMakeLists.txt
├── tests/                      # Each demo: TestClearColor, Test2DTransforms, TestCubes,
│                                #   TestCamera, TestModels, TestCuda, plus the Test/TestMenu base
├── extern/                     # Vendored/fetched deps: glad, glfw, glm, imgui, ImGuiFileDialog,
│                                #   stb_image, assimp
├── res/
│   ├── shader/                  # basic_shader, cube_shader, model_shader, fullscreen_quad
│   ├── models/backpack/         # Sample textured model
│   └── textures/
└── tests/ (top-level)          # Test executable's CMakeLists
```

## Dependencies

- GLFW — window/context/input
- GLAD — OpenGL 4.3 core function loading
- GLM — math (vectors/matrices)
- Dear ImGui (+ backends for GLFW/OpenGL3) — debug/tooling UI
- ImGuiFileDialog — file dialogs
- stb_image — image loading
- Assimp — model loading
- CUDA Toolkit (`cudart`, `cuda_driver`, `curand`) — GPU compute + OpenGL interop

All of the above except the CUDA Toolkit are vendored/built under `extern/` via CMake.

## Setup

1. Requires **CMake ≥ 3.25** and the **CUDA Toolkit** (for `find_package(CUDAToolkit)`).
2. On Linux, **clang** is required — GCC is not supported. Set the compiler explicitly:
   ```bash
   export CC="/usr/bin/clang"
   export CXX="/usr/bin/clang++"
   ```
3. Install X11/build dependencies for GLFW:
   ```bash
   sudo apt install xorg-dev
   ```
4. Clone the repository (with submodules, for the vendored deps under `extern/`):
   ```bash
   git clone --recurse-submodules -b cuda-integration git@github.com:Shouryadip-C/OpenGL-TestBed.git
   cd OpenGL-TestBed
   ```
5. Configure and build:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build . --parallel
   ```
6. Run the executable (`opengl_testbed`) from the build output directory. Resources (`res/`) are copied alongside it automatically as part of the build. The following environment variables need to be set for opengl to use the correct gpu that has cuda support.
    ```
    { "name": "__NV_PRIME_RENDER_OFFLOAD", "value": "1" },
    { "name": "__GLX_VENDOR_LIBRARY_NAME", "value": "nvidia" }
    ```

> `CMAKE_CUDA_ARCHITECTURES` is set to `native`, so CMake will target whatever CUDA-capable GPU is present on the build machine.

## Future Work

- Expand the CUDA interop demos beyond the two current simulations (e.g. particle systems, custom compute-based post-processing)
- Add more rendering techniques as standalone tests (lighting models, shadow mapping, deferred shading)
- Clean up resource teardown on application exit (currently marked `TODO` in `main.cpp`)
- Broaden compiler support beyond clang on Linux
