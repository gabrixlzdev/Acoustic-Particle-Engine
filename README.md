# Acoustic Particle Engine

A high-performance 2D real-time physics simulation of acoustic waves, particle interactions, and acoustic levitation built in **modern C++20** and rendered with **SFML**.

## Architecture (Modular & Data-Oriented Design)

Designed around **Data-Oriented Design (DOD)** and CPU cache locality rather than heavy object-oriented virtual dispatch:

- **`Core/`**: Pure mathematical utilities and contiguous data structures (zero external dependencies).
- **`Physics/`**: Verlet integration solver, uniform-grid spatial partitioning, and acoustic wave equations. Consumes only `Core` and is strictly isolated from graphics.
- **`Graphics/`**: Encapsulates SFML and implements single-draw-call GPU batch rendering via `sf::VertexArray`.
- **`App/`**: Manages the application window, event polling, and the decoupled fixed-timestep simulation loop.

## Tech Stack

- **Language:** C++20 (`g++` / MinGW-w64)
- **Build System:** CMake 3.24+ & Ninja
- **Rendering:** SFML 2.6.2 (fetched and linked statically via CMake `FetchContent`)
- **Testing:** CTest

## Build, Test & Run

```powershell
# 1. Configure CMake and fetch dependencies
cmake -B build -G Ninja

# 2. Compile all modules, tests, and the main executable
cmake --build build

# 3. Run automated unit tests
ctest --test-dir build --output-on-failure

# 4. Run the simulation
.\build\AcousticParticleEngine.exe
```
