#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "ratelimiter.h"

RateLimiter::RateLimiter(double tokenRate)
{
    setTokenRate(tokenRate);
}

void RateLimiter::setTokenRate(double tokenRate)
{
    if (tokenRate > 0.0) {
        m_tokenRate = tokenRate;
        m_lastUpdated = clock::now();
    }
}

void RateLimiter::reserveTokens(unsigned numTokens)
{
    if (numTokens > m_tokenRate) {
        throw std::invalid_argument(
            "Requested number of tokens can not exceed capacity");
    }

    refillTokens();

    if (m_availableTokens < numTokens) {
        unsigned missingTokens = std::ceil(
            (static_cast<double>(numTokens) - m_availableTokens));
        unsigned waitingTime = (
            missingTokens / m_tokenRate) * 1'000'000'000;
        std::this_thread::sleep_for(std::chrono::nanoseconds(waitingTime));
        refillTokens();
    }

    assert (m_availableTokens >= numTokens);
    m_availableTokens -= numTokens;
}

void RateLimiter::refillTokens()
{
    auto currentTime = clock::now();

    auto timeElapsed = std::chrono::duration_cast<
        std::chrono::nanoseconds>(currentTime - m_lastUpdated).count();

    m_availableTokens += std::min(
        timeElapsed * m_tokenRate / 1'000'000'000.0, m_tokenRate);

    m_lastUpdated = currentTime;
}