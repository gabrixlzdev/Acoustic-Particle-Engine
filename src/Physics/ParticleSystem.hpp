#pragma once

#include <cstddef>
// vector is a dynamic array that can grow and shrink in size, it is part of the C++ Standard Library
#include <vector>
#include "Core/Vec2.hpp"

namespace ape {

    struct ParticleSystem{
        std::vector<Vec2> positions; // Vector to store the positions of particles
        std::vector<Vec2> previousPositions; // Vector to store the previous positions of particles
        std::vector<Vec2> accelerations; 
        std::vector<float> radii;
        
        // Constructor that initializes the particle system with a specified maximum number of particles
        explicit ParticleSystem(std::size_t maxParticles = 0){
            reserve(maxParticles); // Reserve memory for the specified number of particles
        }

        //.reserve is a funcition of vector for optimization
        
        // Reserve memory for the specified number of particles
        void reserve(std::size_t capacity){
            positions.reserve(capacity); // Reserve memory for positions
            previousPositions.reserve(capacity); // Reserve memory for previous positions
            accelerations.reserve(capacity); // Reserve memory for accelerations
            radii.reserve(capacity); // Reserve memory for radii
        }

        std::size_t addParticle(Vec2 position, float radius){
            const std::size_t index = positions.size(); // Get the current number of particles
            positions.push_back(position); // Add the new position to the positions vector
            previousPositions.push_back(position); // Add the new position to the previous positions vector
            accelerations.push_back(Vec2{0.0f, 0.0f}); // Initialize acceleration to zero and add it to the accelerations vector
            radii.push_back(radius); // Add the new radius to the radii vector
            return index; // Return the index of the newly added particle
        }

        [[nodiscard]] std::size_t size() const noexcept{
            return positions.size(); // Return the current number of particles
        }

    };
}