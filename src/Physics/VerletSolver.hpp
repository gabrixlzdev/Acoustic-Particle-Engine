#pragma once

#include <algorithm>
#include <cstddef>
#include "Physics/ParticleSystem.hpp"
#include "Grid.hpp"

//
// x_{n+1} = 2*x_n - x_n + a_n * dt^2 --> verlet solver or verlet integration
// x_{n+1} = x_{n} + (x_n - x_{n-1}) + a_n * dt^2
//
namespace ape{

    struct VerletSolver{

        Vec2 gravity{0.0f, 500.0f}; // Downward gravity in pixels/s^2
        //500 pixels/s^2 ~ 9.81 m/s^2
        
        void applyGravity(ParticleSystem& particles) const noexcept{

            const std::size_t count = particles.size(); // Get the number of particles in the system
            // Apply gravity to each particle's acceleration
            for (std::size_t i=0; i< count; ++i){
                particles.accelerations[i] += gravity; // Apply gravity to each particle's acceleration
            }
        }
        
        void integrate(ParticleSystem& particles, float dt) const noexcept{

            const float dtSquared = dt * dt; // Calculate the square of the time step
            const std::size_t count = particles.size(); // Get the number of particles in the system

            for (std::size_t i=0; i< count; ++i){
                // Calculate the displacement of the particle 
                const Vec2 displacement = particles.positions[i] - particles.previousPositions[i];

                // Update the previous position to the current position
                particles.previousPositions[i] = particles.positions[i];

                // Update the current position based on the displacement and acceleration
                //       x_{n+1}       =          x_n           + (x_n - x_{n-1}) +          a_n            *   dt^2
                particles.positions[i] = particles.positions[i] + displacement + particles.accelerations[i] * dtSquared;

                // Reset the acceleration for the next frame
                particles.accelerations[i] = Vec2{0.0f, 0.0f};
            }
        }

        void solveBoundaryCollisions(ParticleSystem& particles, float widh, float height) const noexcept{

            const std::size_t count = particles.size(); 

            //for all the particles in the system
            for (std::size_t i = 0; i < count; ++i){

                const float r = particles.radii[i]; // get the radius of the particle

                // r < positions[i].x < widht - r --> returns positions[i].x
                // positions[i].x < r < widht - r --> returns r
                // r < widht - r < positions[i].x --> returns widht - r
                particles.positions[i].x = std::clamp(particles.positions[i].x, r, widh - r); 
                particles.positions[i].y = std::clamp(particles.positions[i].y, r, height - r);
            }
        }

        void update(ParticleSystem& particles, const Grid& grid, float dt, float widht, float height) const noexcept{
            applyGravity(particles);
            integrate(particles, dt);
            solveCollisions(particles,grid);
            solveBoundaryCollisions(particles, widht, height);
            
        }


        // Solve particle-particle collisions using the spatial grid (O(N) complexity)
    void solveCollisions(ParticleSystem& particles, const Grid& grid) const noexcept 
    {
        const std::size_t count = particles.size();

        for (std::size_t i = 0; i < count; ++i) {
            const Vec2 posI = particles.positions[i];
            const float radiusI = particles.radii[i];

            // 1. Find grid cell coordinates for particle i
            const auto cellX = static_cast<int>(std::max(0.0f, posI.x) / grid.cellSize); //find column 
            const auto cellY = static_cast<int>(std::max(0.0f, posI.y) / grid.cellSize); //find row

            // 2. Check 3x3 neighboring cells (-1 to +1 in X and Y)
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const int nx = cellX + dx;
                    const int ny = cellY + dy;
                    //dx and dy go from −1 to +1, visiting all 9 cells. 
                    //Any particle outside these 9 cells is too far away to physically touch particle i!

                    // Skip cells outside the screen grid boundaries
                    if (nx < 0 || nx >= static_cast<int>(grid.gridWidth) ||
                        ny < 0 || ny >= static_cast<int>(grid.gridHeight)) {
                        continue;
                    }
                    
                    //index = y*width + x
                    const std::size_t neighborCellIdx = static_cast<std::size_t>(ny) * grid.gridWidth + static_cast<std::size_t>(nx);
                    const auto& candidateIndices = grid.cells[neighborCellIdx];

                    // 3. Check collision against particles in this neighbor cell
                    for (std::size_t j : candidateIndices) {
                        // Avoid self-collision and duplicate checks (only check if i < j)
                        if (i >= j) {
                            continue;
                        }

                        const Vec2 delta = particles.positions[i] - particles.positions[j]; // vector j->i
                        const float distSq = delta.lengthSquared(); //d^2 = (r_i + r_j)^2
                        const float minDistance = radiusI + particles.radii[j];

                        // If distance squared < (r1 + r2)^2, particles are overlapping!
                        if (distSq < minDistance * minDistance && distSq > 1e-8f) {
                            const float dist = std::sqrt(distSq); // = d
                            const float overlap = minDistance - dist; // how deep they penetrated into each other (delta)
                            const Vec2 normal = delta * (1.0f / dist); // unit vector lenght = 1 pointing from j to i.

                            // Push particles apart along the collision normal
                            //Since both particles have equal mass, each particle moves half of the overlap:
                            const Vec2 separation = normal * (0.5f * overlap); 
                            particles.positions[i] += separation; // particle i is pushed foreard along +normal vector
                            particles.positions[j] -= separation; // particle j is pushed foreard along -normal vector
                        }
                    }
                }
            }
        }
    }
    };

    
}