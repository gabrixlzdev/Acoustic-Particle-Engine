#include <cassert>
#include "Physics/VerletSolver.hpp"
#include "Physics/Grid.hpp"

int main() {
    // Create a particle system pre-allocated for 10 particles
    ape::ParticleSystem particles{10};

    // Create the Verlet physics solver and a spatial grid
    ape::VerletSolver solver{};
    ape::Grid grid;
    grid.init(800.0f, 600.0f, 16.0f);

    // Test 1: Verify gravity pulls a particle downward (+y) after 1 physics step
    particles.addParticle({100.0f, 100.0f}, 5.0f);

    // Run 1 full physics update: dt = 0.1s inside an 800x600 window
    grid.clear();
    grid.populate(particles.positions);
    solver.update(particles, grid, 0.1f, 800.0f, 600.0f);

    assert(particles.positions[0].x == 100.0f);
    assert(particles.positions[0].y > 100.0f);

    // Test 2: Boundary collision clamps particle inside the floor
    particles.addParticle({100.0f, 700.0f}, 5.0f);
    solver.solveBoundaryCollisions(particles, 800.0f, 600.0f);
    assert(particles.positions[1].y == 595.0f);

    // Test 3: Particle-particle collision pushes two overlapping particles apart
    ape::ParticleSystem pair{2};
    pair.addParticle({100.0f, 100.0f}, 10.0f); // Particle 0 at (100, 100), r = 10
    pair.addParticle({105.0f, 100.0f}, 10.0f); // Particle 1 at (105, 100), r = 10 (overlap = 15px!)

    grid.clear();
    grid.populate(pair.positions);
    solver.solveCollisions(pair, grid);

    // After collision resolution, distance between centers must be >= sum of radii (20.0f)
    const float newDist = (pair.positions[0] - pair.positions[1]).length();
    assert(newDist >= 19.99f);

    return 0;
}