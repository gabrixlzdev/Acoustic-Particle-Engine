#pragma once

#include <algorithm>

// acoustic particle engine = ape namespace
namespace ape {

    struct TimeStepAccumulator{
        float fixedDeltaTime{1.0f / 120.0f}; // Default to 1/120 seconds = 0.0083333 seconds = 8.3333 milliseconds
        float maxFrameTime{0.25f}; // Maximum frame time to avoid spiral of death (in seconds)
        float accumulator{0.0f}; // Accumulated time since last update (in seconds) 

        //add real world elapsed frame time (clamped to maxFrameTime) to the accumulator
        void addFrameTime(float frameTime) noexcept{
            const float clampedTime = std::min(frameTime, maxFrameTime);
            accumulator += clampedTime;
        }

        //consume a fixed time step from the accumulator if enough time has accumulated
        [[nodiscard]] bool  consumeStep() noexcept{
            if (accumulator >= fixedDeltaTime) {
                accumulator -= fixedDeltaTime;
                return true;
            }
            
            return false;    
        }

        // interpolation factor [0.0,1.0] for rendering between the last and next physics update of leftover unsimulated time
        [[nodiscard]] float alpha() const noexcept{
            return accumulator / fixedDeltaTime;
        }

        // reset the accumulator to zero if needed (e.g., when pausing or resetting the simulation)
        void reset() noexcept{
            accumulator = 0.0f;
        }   

    };


}