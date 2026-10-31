#include <SFML/Graphics.hpp>
#include <random>
#include <string>
#include <algorithm>

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
    // Pre-allocate contiguous memory for particles (Struct of Arrays layout)
    constexpr std::size_t INITIAL_PARTICLES = 1500;
    ape::ParticleSystem particles{INITIAL_PARTICLES};

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

    // Visibility toggle: press 'C' to hide/show particles
    bool showParticles = true;

    // Helper lambda function to add a specific number of particles
    auto addParticles = [&](std::size_t amount) {
        // Reserve additional capacity if needed
        particles.reserve(particles.size() + amount);

        std::uniform_real_distribution<float> posX(acousticField.columnMinX - 30.0f, acousticField.columnMaxX + 30.0f);
        std::uniform_real_distribution<float> posY(40.0f, 650.0f);
        std::uniform_real_distribution<float> radius(2.5f, 4.0f);

        for (std::size_t i = 0; i < amount; ++i) {
            particles.addParticle({posX(rng), posY(rng)}, radius(rng));
        }
    };

    // Helper lambda function to remove a specific number of particles
    auto removeParticles = [&](std::size_t amount) {
        const std::size_t countToRemove = std::min(amount, particles.size());
        for (std::size_t i = 0; i < countToRemove; ++i) {
            particles.positions.pop_back();
            particles.previousPositions.pop_back();
            particles.accelerations.pop_back();
            particles.radii.pop_back();
        }
    };

    // Helper lambda function to reset and spawn 1,500 particles
    auto respawnParticles = [&]() {
        particles.positions.clear();
        particles.previousPositions.clear();
        particles.accelerations.clear();
        particles.radii.clear();
        addParticles(INITIAL_PARTICLES);
    };

    // Initial particle spawn
    respawnParticles();

    // 5. On-Screen Display (HUD) Fonts & Text Elements
    sf::Font hudFont;
    // Attempt loading standard Windows TrueType fonts
    bool fontLoaded = hudFont.loadFromFile("C:\\Windows\\Fonts\\consola.ttf") ||
                      hudFont.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // Top-Left Text: Live simulation telemetry
    sf::Text topHudText("", hudFont, 16);
    topHudText.setFillColor(sf::Color(220, 235, 255));
    topHudText.setPosition(20.0f, 20.0f);

    // Bottom-Left Text: Controls and keyboard commands guide
    sf::Text bottomHudText("", hudFont, 13);
    bottomHudText.setFillColor(sf::Color(140, 165, 200));
    bottomHudText.setPosition(20.0f, static_cast<float>(WINDOW_HEIGHT) - 35.0f);
    bottomHudText.setString("[Space] Toggle Acoustic Field  |  [Up/Down] +/- 50 Particles  |  [C] Hide/Show  |  [R] Respawn  |  [Esc] Exit");

    // Main Application Loop
    while (window.isOpen()) {
        // Measure real-world elapsed frame time and accumulate it
        const float frameTime = frameClock.restart().asSeconds();
        timeAccumulator.addFrameTime(frameTime);
        totalSimulationTime += frameTime; // Advance wave propagation animation timer

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
                // 'R' key respawns default particles
                if (event.key.code == sf::Keyboard::R) {
                    respawnParticles();
                }
                // 'C' key toggles particle rendering visibility
                if (event.key.code == sf::Keyboard::C) {
                    showParticles = !showParticles;
                }
                // Up Arrow adds +250 particles
                if (event.key.code == sf::Keyboard::Up) {
                    addParticles(50);
                }
                // Down Arrow removes -250 particles
                if (event.key.code == sf::Keyboard::Down) {
                    removeParticles(50);
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

        // Visual feedback color:
        // Glowing Cyan (100, 220, 255) when acoustic levitation is actively trapping particles.
        // Amber Orange (255, 140, 60) when acoustic field is OFF and gravity takes over.
        const sf::Color particleColor = acousticField.isEnabled 
            ? sf::Color(100, 220, 255) 
            : sf::Color(255, 140, 60);

        // 1. Clear back-buffer with dark navy background (RGB: 15, 17, 26)
        window.clear(sf::Color(15, 17, 26));

        // 2. Draw animated standing acoustic wave column in the background
        renderer.drawAcousticField(
            window,
            acousticField.wavelength,
            acousticField.columnMinX,
            acousticField.columnMaxX,
            static_cast<float>(WINDOW_HEIGHT),
            totalSimulationTime,
            acousticField.isEnabled
        );

        // 3. Draw particles on top of the wave field ONLY if showParticles is true
        if (showParticles && particles.size() > 0) {
            renderer.update(particles.positions, particles.radii, particleColor);
            renderer.draw(window);
        }

        // 4. Render On-Screen Display (HUD) telemetry and controls guide
        if (fontLoaded) {
            // Format live telemetry string
            std::string telemetry = "Particles: " + std::to_string(particles.size()) + "\n" +
                                    "Acoustic Field: " + (acousticField.isEnabled ? "ACTIVE (Trapping)" : "OFF (Gravity Only)") + "\n" +
                                    "Particle Visibility: " + (showParticles ? "VISIBLE" : "HIDDEN (Field Only)");
            topHudText.setString(telemetry);

            window.draw(topHudText);
            window.draw(bottomHudText);
        }

        // 5. Swap back-buffer to the screen (Double Buffering)
        window.display();
    }

    return 0;
}
