#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <gtest/gtest.h>

#include "ratelimiter.h"
#include "stopwatch.h"

TEST(RateLimitTests, RequiredTokensNotAvailableYet)
{
    Stopwatch watch;
    RateLimiter rateLimiter(1.0);

    watch.reset();
    rateLimiter.acquireTokens(1);
    ASSERT_EQ(watch.elapsedMillis(), 1000u);

    watch.reset();
    rateLimiter.acquireTokens(1);
    ASSERT_EQ(watch.elapsedMillis(), 1000u);
}

TEST(RateLimitTests, RequiredTokensLargerThanCapacity)
{
    RateLimiter rateLimiter(1.0);
    EXPECT_THROW(rateLimiter.acquireTokens(2), std::invalid_argument);
}

TEST(RateLimitTests, RequiredTokensImmediatelyAvailable)
{
    RateLimiter rateLimiter(2.0);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    Stopwatch watch;
    rateLimiter.acquireTokens(2);
    ASSERT_EQ(watch.elapsedMillis(), 0u);
}

TEST(RateLimitTests, SomeRequiredTokensNotImmediatelyAvailable)
{
    RateLimiter rateLimiter(2.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Stopwatch watch;
    rateLimiter.acquireTokens(2);
    auto elapsedMillis = watch.elapsedMillis();
    ASSERT_TRUE(elapsedMillis >= 499u && elapsedMillis <= 501u);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
