#include <cassert>
#include "Physics/ParticleSystem.hpp"

int main(){

    //test 1: Create a particle system with a maximum of 100 particles
    ape::ParticleSystem particles{100};// Create a particle system with a maximum of 100 particles
    assert(particles.size() == 0); // Initially, the size should be 0
    assert(particles.positions.capacity() >= 100); // The capacity of the positions vector should be at least 100

    //test 2: Add a particle and check the size and index
    const std::size_t idx = particles.addParticle({10.0f, 20.0f}, 2.5f); // Add a particle at position (1.0, 2.0) with radius 2.5
    assert(particles.size() == 1); // The size should be 1 after adding a particle
    assert(idx == 0); // The index of the added particle should be 0


    //test 3: Verify SoA arrays are synchronized and particle starts at rest
    // The position of the added particle should be (10.0, 20.0)
    assert(particles.positions[0].x == 10.0f && particles.positions[0].y == 20.0f);
    // The previous position of the added particle should also be (10.0, 20.0)
    assert(particles.previousPositions[0].x == 10.0f && particles.previousPositions[0].y == 20.0f);
    // The acceleration of the added particle should be (0.0, 0.0)
    assert(particles.accelerations[0].x == 0.0f && particles.accelerations[0].y == 0.0f);
    // The radius of the added particle should be 2.5
    assert(particles.radii[0] == 2.5f);
} 