#include <cstring>
#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include "tcpsocket.h"
#include "socketerror.h"

constexpr unsigned TcpSocket::DefaultBufferSize;

TcpSocket::TcpSocket(int socketfd)
    : m_socketfd(socketfd)
{
}

TcpSocket::~TcpSocket()
{
    if (m_socketfd != -1) {
        ::close(m_socketfd);
    }
}

void TcpSocket::setUploadSpeedLimit(unsigned bytesPerSec)
{
    m_uploadingSpeedLimiter.setTokenRate(bytesPerSec);
}

unsigned TcpSocket::bytesAvailable() const
{
    int numBytes;
    if (::ioctl(m_socketfd, FIONREAD,
            reinterpret_cast<char *>(&numBytes)) < 0) {
        numBytes = 0;
    }
    return numBytes;
}

bool TcpSocket::read(uint8_t &num) const
{
    return readIntegralValue<uint8_t>(num);
}

bool TcpSocket::read(uint32_t &num) const
{
    bool status = readIntegralValue<uint32_t>(num);
    num = ::ntohl(num);
    return status;
}

bool TcpSocket::read(TcpSocket::buffer_t &buffer, unsigned n) const
{
    if (n != 0) {
        buffer.resize(n);
        return readExactBytes(buffer.data(), buffer.size());
    }

    buffer.resize(DefaultBufferSize);

    auto num_bytes = ::recv(
        m_socketfd, buffer.data(), buffer.size(), 0);

    if (num_bytes < 0) {
        std::cerr << "recv() call failed: " << strerror(errno) << std::endl;
        buffer.clear();
        return false;
    }

    buffer.resize(num_bytes);
    return true;
}

bool TcpSocket::write(uint8_t num)
{
    return send(&num, sizeof(num));
}

bool TcpSocket::write(uint32_t num)
{
    num = ::htonl(num);
    return send(&num, sizeof(num));
}

bool TcpSocket::write(const buffer_t &buffer)
{
    return send(buffer.data(), buffer.size());
}

bool TcpSocket::readExactBytes(char *buffer, size_t size) const
{
    size_t received = 0;
    while (received < size) {
        ssize_t num_bytes = ::recv(
            m_socketfd, buffer + received, size - received, 0);

        if (num_bytes < 0) {
            std::cerr << "recv() call failed: " << strerror(errno)
                      << std::endl;
            return false;
        }

        received += num_bytes;
    }
    return true;
}

TcpSocket::TcpSocket(bool nonblocking)
{
    createSocket(nonblocking);
}

void TcpSocket::createSocket(bool nonblocking)
{
    int flags = SOCK_STREAM;
    if (nonblocking) {
        flags |= SOCK_NONBLOCK;
    }

    m_socketfd = ::socket(AF_INET, flags, 0);
    if (m_socketfd < 0) {
        throw socket_error("socket() call failed");
    }
}

bool TcpSocket::send(const void *buf, size_t len)
{
    m_uploadingSpeedLimiter.acquireTokens(len);
    if (::send(m_socketfd, buf, len, MSG_NOSIGNAL) < 0) {
        std::cerr << "send() call failed: " << strerror(errno)
                  << std::endl;
        return false;
    }
    return true;
}
