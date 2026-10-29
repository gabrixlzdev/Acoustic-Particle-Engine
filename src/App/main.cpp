#include <SFML/Graphics.hpp>
#include "TimeStepAccumulator.hpp"

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
            const float dt = timeAccumulator.fixedDeltaTime;
            (void) dt; // Placeholder for step 4 physics solver
        }

        // Clear back-buffer with a dark background (RGB: 15, 17, 26)
        window.clear(sf::Color(15, 17, 26));

        // Present the rendered frame to the screen (Double Buffering)
        window.display();
    }

    return 0;
}
