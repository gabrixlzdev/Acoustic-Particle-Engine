cmake_minimum_required(VERSION 3.24)
project(AcousticParticleEngine VERSION 0.1.0 LANGUAGES CXX)

# Enforce strict C++20 across all targets
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE IntelliSense / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch SFML 2.6.2 automatically (static linking avoids missing DLL issues on Windows)
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SFML)

# --- Modular Architecture Targets (Data-Oriented Design) ---
# Core: Pure math and data structures (zero external dependencies)
add_library(Core INTERFACE)
target_include_directories(Core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Physics: Solver, spatial grid, and acoustic waves (consumes Core only)
add_library(Physics INTERFACE)
target_include_directories(Physics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Physics INTERFACE Core)

# Graphics: Encapsulates SFML batch rendering (consumes Core + SFML)
add_library(Graphics INTERFACE)
target_include_directories(Graphics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Graphics INTERFACE Core sfml-graphics sfml-window sfml-system)

# App: Main executable managing window and fixed-timestep loop
add_executable(AcousticParticleEngine src/App/main.cpp)
target_link_libraries(AcousticParticleEngine PRIVATE Physics Graphics)

# Enable CTest for automated unit and integration testing
enable_testing()

#compile tests/test_timestep.cpp into TestTimeStep.exe
add_executable(TestTimeStep tests/test_timestep.cpp)

#link core so the test knows where the "src/" folder is
target_link_libraries(TestTimeStep PRIVATE Core)

# Register TestTimeStep.exe with CTest so "ctest" runs it automatically
add_test(NAME TimeStepAccumulatorTest COMMAND TestTimeStep)

#compile tests/test_vec2.cpp into TestVec2.exe
add_executable(TestVec2 tests/test_vec2.cpp)

#link core so the test knows where the "src/" folder is
target_link_libraries(TestVec2 PRIVATE Core)

# Register TestVec2.exe with CTest so "ctest" runs it automatically
add_test(NAME Vec2Test COMMAND TestVec2)
