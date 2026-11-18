# VortexArcana

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![OpenGL](https://img.shields.io/badge/Graphics-OpenGL%204.5-red.svg)](https://www.opengl.org/)
[![.NET 10](https://img.shields.io/badge/Scripting-.NET%2010%20CoreCLR-purple.svg)](https://dotnet.microsoft.com/)
[![ImGui](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![CMake](https://img.shields.io/badge/Build-CMake%203.20%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-brightgreen.svg)](LICENSE)

**VortexArcana** is a high-performance 2D game engine and editor built from scratch in modern **C++20**. It features a custom multi-pass deferred rendering pipeline with dynamic lighting and soft shadows, a cache-friendly Entity-Component System (ECS), an integrated Dear ImGui docking editor, and native **.NET 10 / CoreCLR** runtime embedding via `nethost` for hot-reloadable C# gameplay scripting.

---

## Architectural Highlights

```
+-------------------------------------------------------------------+
|                        VortexArcana Engine                        |
+-------------------------------------------------------------------+
|                                                                   |
|   +-----------------------+             +---------------------+   |
|   |  Entity-Component     |             |  Deferred 2D        |   |
|   |  System (ECS)         |             |  Renderer           |   |
|   |  - Contiguous Arrays  |             |  - Geometry FBOs    |   |
|   |  - Strict RAII        |             |  - SSBO Point Lights|   |
|   |  - Transform / Sprite |             |  - 2-Pass Blur FBOs |   |
|   +-----------+-----------+             +----------+----------+   |
|               |                                    |              |
|               v                                    v              |
|   +-----------------------+             +---------------------+   |
|   |  2D Physics & Sweep   |             |  Dear ImGui Docking |   |
|   |  - AABB Collisions    |             |  - Viewport Picking |   |
|   |  - Impulse Resolution |             |  - Transform Gizmos |   |
|   +-----------+-----------+             |  - RT Diagnostics   |   |
|               |                         +----------+----------+   |
|               |                                    ^              |
|               +------------------+-----------------+              |
|                                  |                                |
|                                  v                                |
|   +-----------------------------------------------------------+   |
|   |           Embedded .NET 10 / CoreCLR Runtime              |   |
|   |        (nethost + Native P/Invoke & Interop Layer)        |   |
|   |   - BaseEntity.cs  |  ScriptHost.cs  |  Hot-Reloading     |   |
|   +-----------------------------------------------------------+   |
+-------------------------------------------------------------------+
```

---

## Core Engine Systems

### 1. Multi-Pass Deferred 2D Rendering
* **Geometry Pass**: Utilizes Framebuffer Objects (FBOs) to capture diffuse albedo, material properties, and normal coordinates into separate render targets.
* **Dynamic Lighting Pass**: Evaluates dynamic 2D point lights via Shader Storage Buffer Objects (SSBOs), decoupling light evaluation complexity from geometric scene density.
* **Soft Dynamic Shadows**: Computes dynamic 2D occlusion maps combined with a separable 2-pass Gaussian blur pass for soft, penumbra-accurate shadow falloff.

### 2. Embedded .NET 10 / CoreCLR Scripting Engine
* **Native Runtime Hosting**: Hosts the Microsoft .NET 10 runtime directly inside native C++ memory using the official `nethost` API.
* **Zero-Crash Interop**: Implements bidirectional function pointer binding and struct marshalling between unmanaged C++ memory and managed C# assemblies (`ScriptingInterface.h`).
* **Live Script Execution**: Gameplay logic inherits from `BaseEntity.cs` in C#, enabling fast developer iteration and dynamic gameplay scripting without recompiling the core C++ engine.

### 3. Cache-Friendly Entity-Component System (ECS)
* Built on contiguous component arrays to maximize CPU L1/L2 cache locality during per-frame update and render loops.
* Strictly follows modern C++ RAII resource ownership, smart pointer lifetimes, and move semantics.

### 4. Integrated Dear ImGui Docking Editor
* **Live Viewport & Gizmos**: Interactive 2D viewport with ray picking, translation/rotation gizmos, and camera controls.
* **Diagnostic Views**: Real-time inspection of intermediate render targets (Albedo FBO, Light Accumulation, Blur buffers).
* **Entity Hierarchy & Inspectors**: Live entity inspection, runtime component modification, and asset browsers.

### 5. High-Speed Serialization & Physics
* **RapidYAML (`ryml`) Integration**: Fast, low-overhead scene serialization and deserialization directly to/from human-readable YAML files.
* **2D Collision Engine**: Fast axis-aligned bounding box (AABB) collision detection, continuous swept tests, and velocity impulse resolution.

---

## Repository Structure

```
VortexArcana/
├── assets/
│   ├── scenes/             # Serialized YAML scene files
│   ├── shaders/            # GLSL vertex, fragment, and post-processing shaders
│   └── textures/           # Sprites, normal maps, and test assets
├── external/               # Submodules & dependencies (GLAD, GLM, ImGui, nethost, ryml, SDL)
├── src/
│   ├── Core/
│   │   ├── Components/     # ECS Components (SpriteRenderer2D, Rigidbody, PointLight, Physics2D)
│   │   ├── CSharp/         # C# Managed Scripts, ScriptHost, and CoreCLR runtime hooks
│   │   ├── Utility/        # Collisions, Math, Shaders, SceneUtility, ScriptingInterface
│   │   ├── Engine.cpp/h    # Main application loop, FBO pipelines, and window management
│   │   └── Entity.cpp/h    # Entity representation and component registry
│   └── main.cpp            # Engine entry point
└── CMakeLists.txt          # Root build configuration
```

---

## Building from Source

### Prerequisites
* **C++ Compiler**: MSVC (Visual Studio 2022+), Clang 14+, or GCC 12+ with full **C++20** support
* **Build System**: [CMake 3.20+](https://cmake.org/download/)
* **Runtime**: [.NET 10 SDK](https://dotnet.microsoft.com/) (for C# scripting compilation)

### Clone & Build
```bash
# Clone with submodules
git clone --recursive https://github.com/DepthPixels/VortexArcana.git
cd VortexArcana

# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build executable
cmake --build build --config Release
```

---

## Author

**Parav Deswal**  
Computer Engineering, University of Illinois at Urbana-Champaign  
GitHub: [@DepthPixels](https://github.com/DepthPixels) | LinkedIn: [parav-deswal](https://www.linkedin.com/in/parav-deswal)
