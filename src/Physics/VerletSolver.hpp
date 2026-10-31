#pragma once

#include <algorithm>
#include <cstddef>
#include "Physics/ParticleSystem.hpp"
#include "Physics/Grid.hpp"
#include "Physics/AcousticField.hpp"

namespace ape {

struct VerletSolver {
    // Constant downward gravitational acceleration (~9.81 m/s^2 scaled to pixels)
    Vec2 gravity{0.0f, 500.0f};

    // Apply downward gravity to all particles: a_total += g
    void applyGravity(ParticleSystem& particles) const noexcept {
        const std::size_t count = particles.size();
        for (std::size_t i = 0; i < count; ++i) {
            particles.accelerations[i] += gravity;
        }
    }

    // Verlet Integration with Stokes viscous air drag damping:
    // v_new = v_old * (1 - gamma * dt)
    // In Verlet, displacement = (x_n - x_{n-1}) represents v * dt.
    // Damping factor (1 - gamma * dt) gradually dissipates kinetic energy so
    // particles stably settle into acoustic levitation nodes instead of oscillating forever.

    void integrate(ParticleSystem& particles, float dt, float damping = 2.5f) const noexcept 
    {
        const float dtSquared = dt * dt; // Pre-calculated dt^2 for performance

        // Clamp damping factor to >= 0 so large dt never reverses velocity
        const float dampingFactor = std::max(0.0f, 1.0f - damping * dt);
        const std::size_t count = particles.size();

        for (std::size_t i = 0; i < count; ++i) {
            // displacement = x_n - x_{n-1} (implicit velocity vector)
            const Vec2 displacement = particles.positions[i] - particles.previousPositions[i];

            // Store current position x_n as previous position x_{n-1} for the next step
            particles.previousPositions[i] = particles.positions[i];

            // Störmer-Verlet formula: x_{n+1} = x_n + (displacement * damping) + a_n * dt^2
            particles.positions[i] = particles.positions[i] + (displacement * dampingFactor) + (particles.accelerations[i] * dtSquared);

            // Reset acceleration to zero for the next frame so forces don't accumulate infinitely
            particles.accelerations[i] = Vec2{0.0f, 0.0f};
        }
    }

    // Keep particles inside the rectangular simulation window (elastic boundary clamping)
    void solveBoundaryCollisions(ParticleSystem& particles, float width, float height) const noexcept {
        const std::size_t count = particles.size();

        for (std::size_t i = 0; i < count; ++i) {
            const float r = particles.radii[i];

            // Clamp particle center to [r, width - r] and [r, height - r]
            // so the particle edge never penetrates past the window walls or floor
            particles.positions[i].x = std::clamp(particles.positions[i].x, r, width - r);
            particles.positions[i].y = std::clamp(particles.positions[i].y, r, height - r);
        }
    }

    // O(N) Particle-Particle Collision Resolution using the Spatial Uniform Grid:
    // For each particle i, we only inspect the 3x3 neighboring cells (-1 to +1 in X and Y).
    void solveCollisions(ParticleSystem& particles, const Grid& grid) const noexcept {
        const std::size_t count = particles.size();

        for (std::size_t i = 0; i < count; ++i) {
            const Vec2 posI = particles.positions[i];
            const float radiusI = particles.radii[i];

            // 1. Identify which grid cell column (X) and row (Y) particle i currently occupies
            const auto cellX = static_cast<int>(std::max(0.0f, posI.x) / grid.cellSize);
            const auto cellY = static_cast<int>(std::max(0.0f, posI.y) / grid.cellSize);

            // 2. Iterate through all 9 cells in the 3x3 neighborhood centered on cell (cellX, cellY)
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const int nx = cellX + dx;
                    const int ny = cellY + dy;

                    // Ignore cells that fall outside the screen boundary
                    if (nx < 0 || nx >= static_cast<int>(grid.gridWidth) ||
                        ny < 0 || ny >= static_cast<int>(grid.gridHeight)) {
                        continue;
                    }

                    // Map 2D cell coordinate (nx, ny) to 1D flattened index: (ny * width) + nx
                    const std::size_t neighborCellIdx = static_cast<std::size_t>(ny) * grid.gridWidth + static_cast<std::size_t>(nx);
                    const auto& candidateIndices = grid.cells[neighborCellIdx];

                    // 3. Test collisions against every particle candidate located in this neighbor cell
                    for (std::size_t j : candidateIndices) {
                        // Avoid testing a particle against itself, and avoid duplicate collision pairs
                        // by enforcing i < j (tests each pair exactly once)
                        if (i >= j) {
                            continue;
                        }

                        // delta = vector pointing from particle j to particle i
                        const Vec2 delta = particles.positions[i] - particles.positions[j];
                        const float distSq = delta.lengthSquared(); // d^2
                        const float minDistance = radiusI + particles.radii[j]; // r_i + r_j

                        // Check if distance squared < (r_i + r_j)^2 (indicates physical overlap)
                        if (distSq < minDistance * minDistance && distSq > 1e-8f) {
                            const float dist = std::sqrt(distSq); // Euclidean distance d
                            const float overlap = minDistance - dist; // Penetration depth delta
                            const Vec2 normal = delta * (1.0f / dist); // Normalized collision unit vector n_hat

                            // Position-Based Dynamics (PBD): Equal mass assumption splits the overlap 50/50.
                            // Particle i is projected forward along +n_hat, particle j is projected backward along -n_hat
                            const Vec2 separation = normal * (0.5f * overlap);
                            particles.positions[i] += separation;
                            particles.positions[j] -= separation;
                        }
                    }
                }
            }
        }
    }

    // Master update pipeline executed on every fixed physics timestep (120 Hz):
    // 1. Gravity Force -> 2. Acoustic Radiation Force -> 3. Verlet Integration with Air Damping
    // -> 4. Spatial Grid Collisions -> 5. Window Boundary Clamping
    void update(ParticleSystem& particles, 
                const Grid& grid, 
                const AcousticField& acousticField, 
                float dt, 
                float width, 
                float height) const noexcept {
        applyGravity(particles);
        acousticField.applyForce(particles);
        integrate(particles, dt, acousticField.damping);
        solveCollisions(particles, grid);
        solveBoundaryCollisions(particles, width, height);
    }
};

} // namespace ape