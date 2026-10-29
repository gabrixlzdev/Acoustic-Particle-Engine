#include <cassert>
#include <cmath>
#include "Core/Vec2.hpp"

int main(){

    //vectors a and b 
    constexpr ape::Vec2 a(3.0f, 4.0f);
    constexpr ape::Vec2 b(1.0f, 2.0f);

    //Test 1: addition, subtraction, scalar multiplication
    constexpr ape::Vec2 sum = a + b;
    assert(sum.x == 4.0f && sum.y == 6.0f); //should be true

    constexpr ape::Vec2 diff = a - b;
    assert(diff.x == 2.0f && diff.y == 2.0f); // should be true

    constexpr ape::Vec2 scaled = a * 2.0f;
    assert(scaled.x == 6.0f && scaled.y == 8.0f); // should be true

    //Test 2: compound assignment operators (+=, -=)
    ape::Vec2 c(5.0f, 5.0f);
    c += b;
    assert(c.x == 6.0f && c.y == 7.0f); // should be true
    c -= b;
    assert(c.x == 5.0f && c.y == 5.0f); // should be true

    //Test 3: LenghtSquare (3^2 + 4^2 = 9 + 16 = 25) and length (sqrt(25) = 5)
    assert(a.lengthSquared() == 25.0f); // should be true
    assert(std::abs(a.length() - 5.0f) < 1e-6f); // should be true

    return 0;
}