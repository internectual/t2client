#pragma once
#include <cstdint>
#include <string>

class Timer {
public:
    Timer();
    void reset();
    double elapsed() const;
    double lap();
    static double now();

private:
    double start = 0;
};
