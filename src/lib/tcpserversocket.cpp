#include <cstring>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "tcpserversocket.h"
#include "socketerror.h"

#include <iostream>

constexpr unsigned TcpServerSocket::QueueLength;

TcpServerSocket::TcpServerSocket(unsigned short port)
    : TcpSocket(true)
{
    bind(port);
    listen();
}

std::unique_ptr<TcpSocket> TcpServerSocket::accept() const
{
    int newSocketFd = ::accept(m_socketfd, nullptr, nullptr);

    if (newSocketFd < 0) {
        return nullptr;
    }

    return std::make_unique<TcpSocket>(newSocketFd);
}

void TcpServerSocket::bind(unsigned short port) const
{
    int enable = 1;
    if (::setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &enable,
            sizeof(int)) < 0) {
        throw socket_error("setsockopt(SO_REUSEADDR) call failed");
    }

    sockaddr_in addr;

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = ::htons(port);

    if (::bind(m_socketfd, reinterpret_cast<sockaddr *>(&addr),
            sizeof(addr)) < 0) {
        throw socket_error("bind() call failed");
    }
}

void TcpServerSocket::listen() const
{
    if (::listen(m_socketfd, QueueLength) < 0) {
        throw socket_error("listen() call failed");
    }
}
