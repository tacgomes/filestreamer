#pragma once

#include <chrono>

class Stopwatch
{
public:
    void reset();
    long unsigned elapsedMillis() const;
    double elapsedSecondsAsDouble() const;

private:
    using clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<clock> m_last = clock::now();
};

inline void Stopwatch::reset()
{
    m_last = clock::now();
}

inline long unsigned Stopwatch::elapsedMillis() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(clock::now() - m_last).count();
}

inline double Stopwatch::elapsedSecondsAsDouble() const
{
    return elapsedMillis() / 1000.0;
}
