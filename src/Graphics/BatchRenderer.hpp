#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <numbers>
#include <SFML/Graphics.hpp>
#include "Core/Vec2.hpp"

namespace ape {

class BatchRenderer {
public:
    // Initialize vertex array using Quads primitive type (4 vertices per particle)
    BatchRenderer() : m_vertices(sf::Quads) {}

    // Update GPU vertex buffer from contiguous memory arrays
    void update(const std::vector<Vec2>& positions, 
                const std::vector<float>& radii, 
                sf::Color color = sf::Color(100, 200, 255)) {
        
        const std::size_t count = positions.size();
        
        // 4 vertices are needed to draw a 2D quad for each particle
        m_vertices.resize(count * 4);

        for (std::size_t i = 0; i < count; ++i) {
            const float x = positions[i].x;
            const float y = positions[i].y;
            const float r = radii[i];
            const std::size_t baseIdx = i * 4; // Calculates starting index in the vertex array for particle i

            // Vertex 0: Top-Left corner
            m_vertices[baseIdx + 0].position = sf::Vector2f(x - r, y - r);
            m_vertices[baseIdx + 0].color    = color;

            // Vertex 1: Top-Right corner
            m_vertices[baseIdx + 1].position = sf::Vector2f(x + r, y - r);
            m_vertices[baseIdx + 1].color    = color;

            // Vertex 2: Bottom-Right corner
            m_vertices[baseIdx + 2].position = sf::Vector2f(x + r, y + r);
            m_vertices[baseIdx + 2].color    = color;

            // Vertex 3: Bottom-Left corner
            m_vertices[baseIdx + 3].position = sf::Vector2f(x - r, y + r);
            m_vertices[baseIdx + 3].color    = color;
        }
    }

    // Draw visual representation of acoustic levitation with counter-propagating traveling waves:
    // Standing wave P(y, t) = cos(k*y - omega*t) [downward wave from top transducer]
    //                       + cos(k*y + omega*t) [upward reflected wave from bottom reflector]
    // Shows the two moving traveling waves physically interfering to trap particles!
    void drawAcousticField(sf::RenderWindow& window, 
                           float wavelength, 
                           float columnMinX,
                           float columnMaxX,
                           float screenHeight, 
                           float totalTime,
                           bool isEnabled) const {
        if (!isEnabled) {
            return;
        }

        constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;
        const float k = twoPi / wavelength;  // Wavenumber k = 2 * pi / lambda
        constexpr float omega = 5.0f;        // Propagation velocity phase speed (omega = 2*pi*f)

        const float columnWidth = columnMaxX - columnMinX;
        const float centerX     = columnMinX + columnWidth * 0.5f;
        const float waveAmp     = columnWidth * 0.35f;

        // -------------------------------------------------------------
        // Layer 1: Hardware Emitter (Top Transducer) and Bottom Reflector
        // -------------------------------------------------------------
        sf::RectangleShape topTransducer(sf::Vector2f(columnWidth + 60.0f, 26.0f));
        topTransducer.setPosition(columnMinX - 30.0f, 0.0f);
        topTransducer.setFillColor(sf::Color(45, 50, 65));
        topTransducer.setOutlineColor(sf::Color(255, 200, 80, 200)); // Amber border for ultrasound source
        topTransducer.setOutlineThickness(2.0f);
        window.draw(topTransducer);

        sf::RectangleShape bottomReflector(sf::Vector2f(columnWidth + 60.0f, 26.0f));
        bottomReflector.setPosition(columnMinX - 30.0f, screenHeight - 26.0f);
        bottomReflector.setFillColor(sf::Color(45, 50, 65));
        bottomReflector.setOutlineColor(sf::Color(160, 100, 255, 200)); // Purple border for acoustic reflector
        bottomReflector.setOutlineThickness(2.0f);
        window.draw(bottomReflector);

        // -------------------------------------------------------------
        // Layer 2: Counter-Propagating Traveling Waves:
        // Wave 1: Downward traveling wave from top transducer: cos(k*y - omega*t)
        // Wave 2: Upward traveling wave from bottom reflector: cos(k*y + omega*t)
        // -------------------------------------------------------------
        constexpr std::size_t numPoints = 350;
        sf::VertexArray downWave(sf::LineStrip, numPoints); // Traveling downward (Amber/Yellow)
        sf::VertexArray upWave(sf::LineStrip, numPoints);   // Traveling upward (Purple/Magenta)
        sf::VertexArray sumWave(sf::LineStrip, numPoints);  // The resultant standing wave (Cyan)

        for (std::size_t i = 0; i < numPoints; ++i) {
            const float y = (static_cast<float>(i) / static_cast<float>(numPoints - 1)) * screenHeight;

            // Wave 1 traveling downward: phase = (k * y - omega * totalTime)
            const float pDown = std::cos(k * y - omega * totalTime);

            // Wave 2 traveling upward (reflected): phase = (k * y + omega * totalTime)
            const float pUp = std::cos(k * y + omega * totalTime);

            // Superposition principle: Standing Wave = Wave 1 + Wave 2
            const float pSum = 0.5f * (pDown + pUp);

            // 1. Downward traveling wave in bright amber
            downWave[i].position = sf::Vector2f(centerX + (pDown * waveAmp * 0.5f), y);
            downWave[i].color    = sf::Color(255, 190, 60, 120);

            // 2. Upward traveling wave in neon purple
            upWave[i].position   = sf::Vector2f(centerX + (pUp * waveAmp * 0.5f), y);
            upWave[i].color      = sf::Color(180, 90, 255, 120);

            // 3. Resultant standing wave in bright glowing cyan
            sumWave[i].position  = sf::Vector2f(centerX + (pSum * waveAmp), y);
            sumWave[i].color     = sf::Color(90, 240, 255, 220);
        }

        // Draw both traveling waves passing each other
        window.draw(downWave);
        window.draw(upWave);

        // Draw resultant standing wave interference
        window.draw(sumWave);

        // -------------------------------------------------------------
        // Layer 3: Acoustic Trapping Nodes (Fixed Planes at lambda/2)
        // Points of zero pressure fluctuation where particles levitate
        // -------------------------------------------------------------
        const float nodeSpacing = wavelength / 2.0f;
        const auto numNodes = static_cast<std::size_t>(screenHeight / nodeSpacing) + 1;

        sf::VertexArray nodeLines(sf::Lines, numNodes * 2);

        for (std::size_t i = 0; i < numNodes; ++i) {
            const float y = static_cast<float>(i) * nodeSpacing;
            const std::size_t baseIdx = i * 2;

            // Thin cyan guide line marking the stationary node
            const sf::Color nodeColor(80, 220, 255, 55);

            nodeLines[baseIdx + 0].position = sf::Vector2f(columnMinX, y);
            nodeLines[baseIdx + 0].color    = nodeColor;

            nodeLines[baseIdx + 1].position = sf::Vector2f(columnMaxX, y);
            nodeLines[baseIdx + 1].color    = nodeColor;
        }
        window.draw(nodeLines);
    }

    // Single draw call sends the entire particle vertex buffer to the GPU
    void draw(sf::RenderWindow& window) const {
        window.draw(m_vertices);
    }

    // Return current vertex count for testing and diagnostics
    [[nodiscard]] std::size_t vertexCount() const noexcept {
        return m_vertices.getVertexCount();
    }

private:
    sf::VertexArray m_vertices; // Contiguous GPU-ready vertex buffer
};

} // namespace ape
