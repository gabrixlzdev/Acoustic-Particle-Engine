#include <cassert>
#include "Physics/VerletSolver.hpp"

int main() {
    // Create a particle system pre-allocated for 10 particles
    ape::ParticleSystem particles{10};

    // Create the Verlet physics solver (default gravity = {0.0f, 500.0f})
    ape::VerletSolver solver{};

    // Test 1: Verify gravity pulls a particle downward (+y) after 1 physics step
    // Spawn particle 0 at (x = 100, y = 100) with radius = 5
    particles.addParticle({100.0f, 100.0f}, 5.0f);

    // Run 1 full physics update: dt = 0.1s inside an 800x600 window
    solver.update(particles, 0.1f, 800.0f, 600.0f);

    // x should remain 100 (no horizontal force), and y must increase (> 100) due to downward gravity
    assert(particles.positions[0].x == 100.0f);
    assert(particles.positions[0].y > 100.0f);

    // Test 2: Verify std::clamp inside solveBoundaryCollisions keeps particles inside the floor
    // Spawn particle 1 at y = 700 (outside the 600px bottom wall) with radius = 5
    particles.addParticle({100.0f, 700.0f}, 5.0f);

    // Run boundary collision check for an 800x600 box
    solver.solveBoundaryCollisions(particles, 800.0f, 600.0f);

    // Maximum allowed y is (height - radius) = 600.0f - 5.0f = 595.0f
    assert(particles.positions[1].y == 595.0f);

    return 0;
}