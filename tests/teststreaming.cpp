#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "fileuploader.h"
#include "filereceiver.h"
#include "stopwatch.h"

void createTestFile(const std::string &filename, size_t numBytes)
{
    std::ofstream infile(filename, std::ios::binary);

    std::mt19937 gen{ std::random_device()() };
    std::uniform_int_distribution<> dis(0, 255);

    std::generate_n(
        std::ostream_iterator<char>(infile, ""),
        numBytes, [&]{ return dis(gen); });
}

uint32_t calculateChecksum(std::string &filename)
{
    std::ifstream infile(filename, std::ios::binary);

    unsigned shift = 0;
    uint32_t checksum = 0;
    do {
        uint32_t c = infile.get();
        checksum += (c << shift);
        shift += 8;
        if (shift == 32) {
            shift = 0;
        }
    } while (infile);

    return checksum;
}

constexpr long operator""_Mb( unsigned long long n)
{
    return n * 1024L * 1024L;
}

TEST(StreamingTests, TestStreaming)
{
    std::string filename("testfile10Mb");
    createTestFile(filename, 10_Mb);

    FileReceiver receiver(9090);
    FileUploader uploader("127.0.0.1", 9090);

    std::thread receiverThread(&FileReceiver::start, &receiver);
    std::thread uploaderThread(&FileUploader::upload, &uploader, filename);

    uploaderThread.join();

    receiver.stop();
    receiverThread.join();

    auto checkSumOriginal = calculateChecksum(filename);
    auto receivedFilename = filename + ".received";
    auto checkSumCopied = calculateChecksum(receivedFilename);
    ASSERT_EQ(checkSumOriginal, checkSumCopied);

    std::remove(filename.c_str());
}

TEST(StreamingTests, RestrictedUploadSpeed)
{
    std::string filename("testfile10Mb");
    createTestFile(filename, 10_Mb);

    FileReceiver receiver(9090);
    FileUploader uploader("127.0.0.1", 9090, 1_Mb);

    std::thread receiverThread(&FileReceiver::start, &receiver);
    std::thread uploaderThread(&FileUploader::upload, &uploader, filename);

    // The transfer should take 10 seconds to complete.
    // Allow a margin of error of 500 milliseconds.
    Stopwatch watch;
    uploaderThread.join();
    auto elapsedMillis = watch.elapsedMillis();
    ASSERT_TRUE(elapsedMillis > 9500 && elapsedMillis < 10500);

    receiver.stop();
    receiverThread.join();

    auto checkSumOriginal = calculateChecksum(filename);
    auto receivedFilename = filename + ".received";
    auto checkSumCopied = calculateChecksum(receivedFilename);
    ASSERT_EQ(checkSumOriginal, checkSumCopied);

    std::remove(filename.c_str());
}

TEST(StreamingTests, ResumeUploading)
{
    FileReceiver receiver(9090);
    FileUploader uploader("127.0.0.1", 9090, 1_Mb);

    std::string filename("testfile10Mb");
    createTestFile(filename, 10_Mb);

    std::thread receiverThread(&FileReceiver::start, &receiver);

    Stopwatch watch;
    std::thread uploaderThread(&FileUploader::upload, &uploader, filename);

    // Stop the receiver server in the middle of the transfer
    std::this_thread::sleep_for(std::chrono::seconds(5));
    receiver.stopNow();
    receiverThread.join();

    // Restart the receiver server
    receiverThread = std::thread(&FileReceiver::start, &receiver);

    // Wait for file upload to complete
    uploaderThread.join();

    auto elapsedMillis = watch.elapsedMillis();
    ASSERT_TRUE(elapsedMillis > 10000 && elapsedMillis < 11500);

    receiver.stop();
    receiverThread.join();

    auto checkSumOriginal = calculateChecksum(filename);
    auto receivedFilename = filename + ".received";
    auto checkSumCopied = calculateChecksum(receivedFilename);
    ASSERT_EQ(checkSumOriginal, checkSumCopied);

    std::remove(filename.c_str());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
