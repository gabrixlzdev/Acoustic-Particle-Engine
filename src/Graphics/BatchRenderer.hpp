#pragma once

#include <cstddef>
#include <vector>
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
            const std::size_t baseIdx = i * 4; //Calculates the starting index in the vertex array for particle i.

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

    // Single draw call sends the entire vertex buffer to the GPU
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