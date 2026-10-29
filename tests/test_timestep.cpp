#include <cassert>
#include <cmath>
#include "App/TimeStepAccumulator.hpp"

int main(){

    ape::TimeStepAccumulator acc{};

    //test 1: one 60hz frame (1/60s) must produce exactlu 2 physics steps of 120hz (1/120s)
    acc.addFrameTime(1.0f / 60.0f); // 1/60 seconds
    assert(acc.consumeStep() == true); // Should consume a step
    assert(acc.consumeStep() == true); // Should consume another step
    assert(acc.consumeStep() == false); // Should not consume a step, not enough time accumulated 120-60-60 = 0.0083333 seconds left in the accumulator

    //test 2: Reset clears the accumulator
    acc.reset();
    assert(acc.accumulator == 0.0f); // Accumulator should be reset to zero
    
    //test 3: Spiral of Death clamp (5.0s freeze must clamp to maxFrameTime of 0.25s
    acc.reset();
    acc.addFrameTime(5.0f); // 5 seconds
    assert(std::abs(acc.accumulator - acc.maxFrameTime) < 1e-6f); // Should be clamped to maxFrameTime
}
