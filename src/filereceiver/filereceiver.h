#pragma once

#include <atomic>
#include <chrono>
#include <memory>

class TcpSocket;
class TcpServerSocket;

class FileReceiver
{
public:
    explicit FileReceiver(unsigned port);
    ~FileReceiver();
    FileReceiver(const FileReceiver &) = delete;
    FileReceiver &operator=(const FileReceiver &) = delete;

    void start();
    void stop();
    void stopNow();

private:
    void process(std::unique_ptr<TcpSocket> socket) const;

    enum class Action {Start, Stop, StopNow};
    std::atomic<Action> m_action {Action::Stop};

    std::unique_ptr<TcpServerSocket> m_socket;

    constexpr static auto PollingTime = std::chrono::milliseconds(200);
    constexpr static uint32_t MaxBytesNotAcknowledged = 1u * 1024 * 1024;
};