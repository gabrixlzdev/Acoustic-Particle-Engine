#pragma once

#include <cmath>

namespace ape {

    
    struct  Vec2 {
        //let storage 8bytes (4 each) in memory
        float x{0.0f};
        float y{0.0f};

        // constexpr -> this function can be evaluated at compile time improving performance
        // teaching the compiler how to operate on Vec2 objects
        // how to do addition, subtraction, multiplication, division, etc.

        constexpr Vec2 operator+ (const Vec2& other) const noexcept{
            return {x + other.x, y + other.y}; // Addition of two Vec2 objects
        }

        constexpr Vec2 operator- (const Vec2& other) const noexcept{
            return {x - other.x, y - other.y}; // Subtraction of two Vec2 objects
        }

        constexpr Vec2 operator* (float scalar) const noexcept{
            return {x * scalar, y * scalar}; // Scalar multiplication
        }

        constexpr Vec2& operator+= (const Vec2& other) noexcept{
            x += other.x;
            y += other.y;
            return *this; // Compound addition assignment
        }

        constexpr Vec2& operator-= (const Vec2& other) noexcept{
            x -= other.x;
            y -= other.y;
            return *this; // Compound subtraction assignment
        }

        [[nodiscard]] constexpr float lengthSquared() const noexcept{
            return x * x + y * y; // Calculate the squared length of the vector
        }

        [[nodiscard]] float length() const noexcept{
            return std::sqrt(lengthSquared()); // Calculate the length of the vector
        }
    };
}