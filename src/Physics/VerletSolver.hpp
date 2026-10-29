#pragma once

#include <algorithm>
#include <cstddef>
#include "Physics/ParticleSystem.hpp"

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

        void update(ParticleSystem& particles, float dt, float widht, float height) const noexcept{
            applyGravity(particles);
            integrate(particles, dt);
            solveBoundaryCollisions(particles, widht, height);
        }
    };
}