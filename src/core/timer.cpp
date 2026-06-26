#include "core/timer.h"
#include <chrono>

Timer::Timer() { reset(); }

void Timer::reset() {
    start = now();
}

double Timer::elapsed() const {
    return now() - start;
}

double Timer::lap() {
    double t = now();
    double d = t - start;
    start = t;
    return d;
}

double Timer::now() {
    static auto epoch = std::chrono::steady_clock::now();
    auto t = std::chrono::steady_clock::now() - epoch;
    return std::chrono::duration<double>(t).count();
}
