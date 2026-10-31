#include <SFML/Graphics.hpp>
#include <random>

#include "App/TimeStepAccumulator.hpp"
#include "Physics/ParticleSystem.hpp"
#include "Physics/VerletSolver.hpp"
#include "Graphics/BatchRenderer.hpp"
#include "Physics/Grid.hpp"

int main() {
    // Compile-time constants (zero runtime memory overhead)
    constexpr unsigned int WINDOW_WIDTH  = 1280;
    constexpr unsigned int WINDOW_HEIGHT = 720;

    // Create the main window on the stack
    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Acoustic Particle Engine (C++20)"
    );
    window.setFramerateLimit(60);

    //Create accumulator and clock on the stack

    sf::Clock frameClock; // Clock to measure frame time
    ape::TimeStepAccumulator timeAccumulator{};

    // Physics solver & Particle System
    constexpr std::size_t NUM_PARTICLES = 1000;
    ape::ParticleSystem particles{NUM_PARTICLES};
    ape::VerletSolver solver{};

    //create grid;    
    ape::Grid grid;
    grid.init(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 16.0f);

    //Graphics Batch Renderer
    ape::BatchRenderer renderer;

    //spawn 1000 particles randomly across the top of the window
    std::mt19937 rng(42); // deterministic random seed
    std::uniform_real_distribution<float> posX(20.0f,static_cast<float>(WINDOW_WIDTH) - 20.0f);
    std::uniform_real_distribution<float> posY(20.0f,300.0f);
    std::uniform_real_distribution<float> radius(2.0f,4.0f);
    


    for (std::size_t i = 0; i < NUM_PARTICLES; ++i)
    {
        particles.addParticle({posX(rng),posY(rng)}, radius(rng));
    }
    

    // Main application loop
    while (window.isOpen()) {
        // Measure the elapsed frame time and feed accumulator
        const float frameTime = frameClock.restart().asSeconds();
        timeAccumulator.addFrameTime(frameTime);

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        while (timeAccumulator.consumeStep()){
            grid.clear();
            grid.populate(particles.positions);
            solver.update(
             particles,
             grid,
             timeAccumulator.fixedDeltaTime,
             static_cast<float>(WINDOW_WIDTH),
             static_cast<float>(WINDOW_HEIGHT)
            );
        }

        //Render: sync vertex buffer and draw in 1 single call
        renderer.update(particles.positions, particles.radii);

        // Clear back-buffer with a dark background (RGB: 15, 17, 26)
        window.clear(sf::Color(15, 17, 26));

        renderer.draw(window); // Sends all 1,000 particles to the screen in 1 single draw call!

        // Present the rendered frame to the screen (Double Buffering)
        window.display();
    }

    return 0;
}
