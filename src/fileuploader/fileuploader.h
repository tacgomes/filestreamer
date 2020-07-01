#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

class TcpClientSocket;

class FileUploader
{
public:
    FileUploader(
        const std::string &host, unsigned short port, unsigned limitRate = 0);
    FileUploader(const FileUploader &) = delete;
    FileUploader &operator=(const FileUploader &) = delete;

    void upload(const std::string &filename);

private:
    bool sendHeader(std::unique_ptr<TcpClientSocket> &socket,
        const std::string &filename, uint32_t filesize, uint32_t offset = 0);

    void reconnect(std::unique_ptr<TcpClientSocket> &socket,
        const std::string &filename, uint32_t filesize, uint32_t offset);

    std::string m_host;
    unsigned short m_port;
    unsigned m_limitRate;

    constexpr static unsigned BufferSize = 1460;
    constexpr static auto ReconnectionWaitingTime = std::chrono::seconds(1);
};