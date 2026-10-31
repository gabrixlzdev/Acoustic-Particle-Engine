#include <SFML/Graphics.hpp>
#include <random>

#include "App/TimeStepAccumulator.hpp"
#include "Physics/ParticleSystem.hpp"
#include "Physics/VerletSolver.hpp"
#include "Physics/Grid.hpp"
#include "Physics/AcousticField.hpp"
#include "Graphics/BatchRenderer.hpp"

int main() {
    // Window resolution constants
    constexpr unsigned int WINDOW_WIDTH  = 1280;
    constexpr unsigned int WINDOW_HEIGHT = 720;

    // Create the OS rendering window (capped at 60 FPS monitor refresh rate)
    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Acoustic Particle Engine - Acoustic Levitation (C++20)"
    );
    window.setFramerateLimit(60);

    // 1. Time Accumulator: Decouples rendering (60 Hz) from deterministic physics (120 Hz)
    ape::TimeStepAccumulator timeAccumulator{};
    sf::Clock frameClock; // High-precision OS frame stopwatch
    float totalSimulationTime = 0.0f; // Running simulation time counter for wave animation

    // 2. Physics Subsystems:
    // Pre-allocate contiguous memory for 1,500 particles (Struct of Arrays layout)
    constexpr std::size_t NUM_PARTICLES = 1500;
    ape::ParticleSystem particles{NUM_PARTICLES};

    // Verlet integrator & solver (applies gravity, acoustic forces, air damping, and collisions)
    ape::VerletSolver solver{};

    // Spatial uniform grid for O(N) particle collision detection
    ape::Grid grid;
    grid.init(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 16.0f);

    // 3. Acoustic Levitation Field:
    // Standing wave parameters based on Andrade et al. (RBEF 2015):
    // - amplitude = 2200 px/s^2 (overcomes downward gravity of 500 px/s^2 to lift particles)
    // - wavelength = 90 px (produces horizontal levitation planes every lambda/2 = 45 px)
    // - columnMinX = 440 px, columnMaxX = 840 px (centered 400 px ultrasound beam)
    ape::AcousticField acousticField;
    acousticField.amplitude = 2200.0f;
    acousticField.wavelength = 90.0f;
    acousticField.damping = 3.0f;
    acousticField.columnMinX = 440.0f;
    acousticField.columnMaxX = 840.0f;

    // 4. GPU Batch Renderer: Draws all particles in a single draw call via sf::VertexArray
    ape::BatchRenderer renderer;

    // Pseudo-random number generator for spawning particles
    std::mt19937 rng(42);

    // Helper lambda function to reset and spawn particles inside and around the acoustic chamber
    auto respawnParticles = [&]() {
        particles.positions.clear();
        particles.previousPositions.clear();
        particles.accelerations.clear();
        particles.radii.clear();

        // Spawn particles concentrated around the acoustic levitator column
        std::uniform_real_distribution<float> posX(380.0f, 900.0f);
        std::uniform_real_distribution<float> posY(40.0f, 650.0f);
        std::uniform_real_distribution<float> radius(2.5f, 4.0f);

        for (std::size_t i = 0; i < NUM_PARTICLES; ++i) {
            particles.addParticle({posX(rng), posY(rng)}, radius(rng));
        }
    };

    // Initial particle spawn
    respawnParticles();

    // Main Application Loop
    while (window.isOpen()) {
        // Measure real-world elapsed frame time and accumulate it
        const float frameTime = frameClock.restart().asSeconds();
        timeAccumulator.addFrameTime(frameTime);
        totalSimulationTime += frameTime; // Advance the wave animation clock

        // Process OS and keyboard input events
        sf::Event event{};
        while (window.pollEvent(event)) {
            // Window close button (X)
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Keyboard controls
            if (event.type == sf::Event::KeyPressed) {
                // Escape key closes application
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                // Spacebar toggles acoustic standing wave ON / OFF
                if (event.key.code == sf::Keyboard::Space) {
                    acousticField.isEnabled = !acousticField.isEnabled;
                }
                // 'R' key respawns all particles
                if (event.key.code == sf::Keyboard::R) {
                    respawnParticles();
                }
            }
        }

        // Fixed-Timestep Physics Simulation Loop (runs at 120 Hz, dt = 1/120s)
        while (timeAccumulator.consumeStep()) {
            // Re-populate the spatial partition grid with latest particle positions
            grid.clear();
            grid.populate(particles.positions);

            // Execute full physics update (Gravity + Acoustic Force + Damped Verlet + Grid Collisions + Boundaries)
            solver.update(
                particles,
                grid,
                acousticField,
                timeAccumulator.fixedDeltaTime,
                static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT)
            );
        }

        // Visual feedback:
        // Glowing Cyan (100, 220, 255) when acoustic levitation is actively trapping particles.
        // Amber Orange (255, 140, 60) when acoustic field is OFF and gravity takes over.
        const sf::Color particleColor = acousticField.isEnabled 
            ? sf::Color(100, 220, 255) 
            : sf::Color(255, 140, 60);

        // Update GPU vertex buffer with current coordinates and color
        renderer.update(particles.positions, particles.radii, particleColor);

        // Clear back-buffer with dark navy background (RGB: 15, 17, 26)
        window.clear(sf::Color(15, 17, 26));

        // Draw animated standing acoustic wave column (transducer, reflector, wave lobes, and nodes)
        renderer.drawAcousticField(
            window,
            acousticField.wavelength,
            acousticField.columnMinX,
            acousticField.columnMaxX,
            static_cast<float>(WINDOW_HEIGHT),
            totalSimulationTime,
            acousticField.isEnabled
        );

        // Submit single draw call to the GPU for all particles
        renderer.draw(window);

        // Swap back-buffer to the screen (Double Buffering)
        window.display();
    }

    return 0;
}
