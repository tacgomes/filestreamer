#pragma once

#include <chrono>

class RateLimiter
{
public:
    explicit RateLimiter(double tokenRate = -1.0);
    RateLimiter(const RateLimiter &) = delete;
    RateLimiter &operator=(const RateLimiter &) = delete;

    void setTokenRate(double tokenRate);
    void acquireTokens(unsigned numTokens);

private:
    void reserveTokens(unsigned numTokens);
    void refillTokens();

    double m_tokenRate;
    double m_availableTokens = 0.0;

    using clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<clock> m_lastUpdated {};
};

inline void RateLimiter::acquireTokens(unsigned numTokens)
{
    if (m_lastUpdated.time_since_epoch().count() == 0) {
        return;
    }
    return reserveTokens(numTokens);
}
