#include <cassert>
#include <vector>
#include "Graphics/BatchRenderer.hpp"
#include "Core/Vec2.hpp"

int main() {
    // 1. Create a BatchRenderer instance
    ape::BatchRenderer renderer;

    // Initially, there are no vertices allocated
    assert(renderer.vertexCount() == 0);

    // 2. Prepare sample particle data: 3 particles
    std::vector<ape::Vec2> positions = {
        {10.0f, 10.0f},
        {50.0f, 50.0f},
        {90.0f, 90.0f}
    };
    std::vector<float> radii = {2.0f, 3.0f, 4.0f};

    // 3. Update the vertex buffer
    renderer.update(positions, radii);

    // 4. Verify vertex count: 3 particles * 4 vertices = 12 vertices
    assert(renderer.vertexCount() == 12);

    return 0;
}