#pragma once

#include <SDL.h>

struct FixedTimestepClock {
    Uint64 last_ticks = 0;
    double freq = 0.0;
    double accumulator = 0.0;
};

inline void InitClock(FixedTimestepClock& clock) {
    clock.last_ticks = SDL_GetPerformanceCounter();
    clock.freq = static_cast<double>(SDL_GetPerformanceFrequency());
    clock.accumulator = 0.0;
}

inline double TickClock(FixedTimestepClock& clock) {
    Uint64 now = SDL_GetPerformanceCounter();
    double frame_dt = (now - clock.last_ticks) / clock.freq;
    clock.last_ticks = now;
    if (frame_dt > 0.25) {
        frame_dt = 0.25;
    }
    clock.accumulator += frame_dt;
    return frame_dt;
}

inline bool ConsumeFixedStep(FixedTimestepClock& clock, double fixed_dt) {
    if (clock.accumulator >= fixed_dt) {
        clock.accumulator -= fixed_dt;
        return true;
    }
    return false;
}
