#include <SFML/Graphics.hpp>

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

    // Main application loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }

        // Clear back-buffer with a dark background (RGB: 15, 17, 26)
        window.clear(sf::Color(15, 17, 26));

        // Present the rendered frame to the screen (Double Buffering)
        window.display();
    }

    return 0;
}
