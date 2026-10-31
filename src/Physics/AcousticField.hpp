#pragma once

#include <cmath>
#include <numbers>
#include "Core/Vec2.hpp"
#include "Physics/ParticleSystem.hpp"

namespace ape {

struct AcousticField {
    // Peak acoustic radiation force acceleration (in pixels/s^2)
    // Needs to exceed downward gravity (500 px/s^2) to achieve stable levitation
    float amplitude{2200.0f};  
    
    // Standing wave wavelength in pixels:
    // Trapping pressure nodes occur at half-wavelength intervals: lambda / 2 = 45 px
    float wavelength{90.0f};   

    // Air drag damping (gamma) to absorb particle kinetic energy into the nodes
    float damping{3.0f};       

    // Master on/off toggle for the acoustic field
    bool isEnabled{true};      

    // Horizontal bounds of the ultrasound beam emitted between transducer and reflector
    // Only particles within [columnMinX, columnMaxX] experience acoustic radiation forces
    float columnMinX{440.0f};  
    float columnMaxX{840.0f};  

    // Calculate acoustic radiation force at position (x, y)
    // Based on Marco Andrade et al. (RBEF 2015), Eq. (10):
    // F_z = (5 * pi * R^3 * A^2 * k) / (6 * rho * c^2) * sin(2 * k * z)
    [[nodiscard]] Vec2 calculateForce(float x, float y) const noexcept {
        // Outside the ultrasound beam column or if disabled, acoustic force is zero (particles fall freely under gravity)
        if (!isEnabled || x < columnMinX || x > columnMaxX) {
            return {0.0f, 0.0f};
        }

        // Wavenumber k = 2 * pi / lambda
        constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;
        const float k = twoPi / wavelength;

        // Vertical Gor'kov acoustic radiation force:
        // Result of interference between downward wave (transducer) and upward reflected wave (reflector)
        // Traps particles stably at intervals of lambda / 2
        const float forceY = -amplitude * std::sin(2.0f * k * y);

        // Acoustic radial radiation force:
        // Pressure gradient pinches particles toward the center line of the acoustic beam
        const float centerX = (columnMinX + columnMaxX) * 0.5f;
        const float radialForceX = -4.0f * (x - centerX);

        return {radialForceX, forceY};
    }

    // Apply acoustic force to all particles in the system
    void applyForce(ParticleSystem& particles) const noexcept {
        if (!isEnabled) {
            return;
        }

        const std::size_t count = particles.size();
        for (std::size_t i = 0; i < count; ++i) {
            particles.accelerations[i] += calculateForce(particles.positions[i].x, particles.positions[i].y);
        }
    }
};

} // namespace ape
