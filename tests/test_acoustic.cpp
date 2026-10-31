#include <cassert>
#include <cmath>
#include <numbers>
#include "Physics/AcousticField.hpp"
#include "Physics/ParticleSystem.hpp"

int main() {
    ape::AcousticField field;
    field.amplitude = 1000.0f;
    field.wavelength = 80.0f;
    field.columnMinX = 0.0f;
    field.columnMaxX = 500.0f;
    field.isEnabled = true;

    // Test 1: At y = 0, sin(0) = 0 -> Force must be zero
    const ape::Vec2 forceAtZero = field.calculateForce(250.0f, 0.0f);
    assert(std::abs(forceAtZero.y) < 1e-4f);

    // Test 2: At y = lambda / 8 (10px), 2 * k * y = pi / 2
    // sin(pi / 2) = 1.0 -> Force must be maximum negative (-amplitude = -1000.0f)
    const float yMaxNegative = field.wavelength / 8.0f;
    const ape::Vec2 forcePeak = field.calculateForce(250.0f, yMaxNegative);
    assert(std::abs(forcePeak.y - (-1000.0f)) < 1e-3f);

    // Test 3: At y = lambda / 4 (20px), 2 * k * y = pi
    // sin(pi) = 0 -> Stable acoustic node (zero vertical force plane)
    const float yNode = field.wavelength / 4.0f;
    const ape::Vec2 forceNode = field.calculateForce(250.0f, yNode);
    assert(std::abs(forceNode.y) < 1e-3f);

    // Test 4: Outside acoustic beam column (x = 800px), force is zero
    assert(field.calculateForce(800.0f, yMaxNegative).y == 0.0f);

    // Test 5: When disabled, force is always zero
    field.isEnabled = false;
    assert(field.calculateForce(250.0f, yMaxNegative).y == 0.0f);

    return 0;
}
