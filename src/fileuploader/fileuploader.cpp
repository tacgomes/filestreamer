#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <thread>

#include "fileuploader.h"
#include "tcpclientsocket.h"
#include "socketerror.h"
#include "stopwatch.h"

constexpr unsigned FileUploader::BufferSize;
constexpr std::chrono::seconds FileUploader::ReconnectionWaitingTime;

FileUploader::FileUploader(
    const std::string &host, unsigned short port, unsigned limitRate)
    : m_host(host)
    , m_port(port)
    , m_limitRate(limitRate)
{
}

void FileUploader::upload(const std::string &filename)
{
    std::ifstream infile(filename, std::ifstream::binary);
    if (!infile.is_open()) {
        throw std::runtime_error("File does not exist");
    }

    const auto startPos = infile.tellg();
    infile.seekg(0, std::ios::end);
    const auto endPos = infile.tellg();
    uint32_t filesize = endPos - startPos;
    infile.seekg(0, std::ios::beg);

    auto socket = std::make_unique<TcpClientSocket>(m_host, m_port);

    if (!sendHeader(socket, filename, filesize)) {
        throw socket_error("Failed to send the header");
    }

    auto bufferSize = BufferSize;

    if (m_limitRate) {
        socket->setUploadSpeedLimit(m_limitRate);
        bufferSize = std::min(m_limitRate, bufferSize);
    }

    Stopwatch watch;
    uint32_t bytesSent = 0;
    uint32_t bytesAcknowledged = 0;
    TcpSocket::buffer_t buffer(bufferSize);

    while (infile) {
        infile.read(buffer.data(), buffer.size());
        buffer.resize(infile.gcount());

        if (socket->write(buffer)) {
            bytesSent += buffer.size();
        } else {
            infile.seekg(bytesAcknowledged);
            reconnect(socket, filename, filesize, bytesAcknowledged);
        }

        if (socket->bytesAvailable() > 0) {
            uint32_t newBytesAcknowledged;
            if (socket->read(newBytesAcknowledged)) {
                bytesAcknowledged = newBytesAcknowledged;
            } else {
                std::cerr << "WARNING: failed to read acknowledgement\n";
            }
        }

        if (infile.eof()) {
            while (filesize != bytesAcknowledged) {
                uint32_t newBytesAcknowledged;
                if (socket->read(newBytesAcknowledged)) {
                    bytesAcknowledged = newBytesAcknowledged;
                } else {
                    std::cerr << "WARNING: failed to read acknowledgement\n";
                    infile.seekg(bytesAcknowledged);
                    reconnect(socket, filename, filesize, bytesAcknowledged);
                }
            }
        }
    }

    auto seconds = watch.elapsedSecondsAsDouble();

    std::cout << "Elapsed time: " << seconds << " seconds\n";

    std::cout << "Bytes transfered: " << bytesSent << " bytes\n";

    std::cout << "Average upload speed: "
              << std::lround(bytesSent / seconds) << " bytes/sec\n";
}

bool FileUploader::sendHeader(
    std::unique_ptr<TcpClientSocket> &socket, const std::string &filename,
    std::uint32_t filesize, std::uint32_t offset)
{
    std::uint8_t filenameLength = filename.size();
    TcpSocket::buffer_t filenameBytes(filename.begin(), filename.end());

    bool res = true;
    res &= socket->write(filenameLength);
    res &= socket->write(filenameBytes);
    res &= socket->write(filesize);
    res &= socket->write(offset);
    return res;
}

void FileUploader::reconnect(
    std::unique_ptr<TcpClientSocket> &socket, const std::string &filename,
    uint32_t filesize, uint32_t offset)
{
    do {
        std::cout << "Reconnecting...\n";
        socket.reset();
        try {
            socket.reset(new TcpClientSocket(m_host, m_port));
            if (!sendHeader(socket, filename, filesize, offset)) {
                socket.reset();
            }
            socket->setUploadSpeedLimit(m_limitRate);
        } catch (const socket_error &) {
            std::this_thread::sleep_for(ReconnectionWaitingTime);
        }
    } while (socket == nullptr);

    std::cout << "Reconnected\n";
}
