#include <cassert>
#include "Physics/VerletSolver.hpp"
#include "Physics/Grid.hpp"
#include "Physics/AcousticField.hpp"

int main() {
    ape::ParticleSystem particles{10};
    ape::VerletSolver solver{};
    ape::Grid grid;
    grid.init(800.0f, 600.0f, 16.0f);

    ape::AcousticField acousticField;
    acousticField.isEnabled = false; // Disable acoustic field for basic gravity test

    // Test 1: Gravity pulls particle downward (+y)
    particles.addParticle({100.0f, 100.0f}, 5.0f);
    grid.clear();
    grid.populate(particles.positions);
    solver.update(particles, grid, acousticField, 0.1f, 800.0f, 600.0f);

    assert(particles.positions[0].x == 100.0f);
    assert(particles.positions[0].y > 100.0f);

    // Test 2: Boundary collision clamps particle inside the floor
    particles.addParticle({100.0f, 700.0f}, 5.0f);
    solver.solveBoundaryCollisions(particles, 800.0f, 600.0f);
    assert(particles.positions[1].y == 595.0f);

    // Test 3: Particle-particle collision pushes overlapping particles apart
    ape::ParticleSystem pair{2};
    pair.addParticle({100.0f, 100.0f}, 10.0f);
    pair.addParticle({105.0f, 100.0f}, 10.0f);

    grid.clear();
    grid.populate(pair.positions);
    solver.solveCollisions(pair, grid);

    const float newDist = (pair.positions[0] - pair.positions[1]).length();
    assert(newDist >= 19.99f);

    return 0;
}