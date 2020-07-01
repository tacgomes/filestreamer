#include <cstring>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "tcpclientsocket.h"
#include "socketerror.h"

TcpClientSocket::TcpClientSocket(const std::string &host, unsigned short port)
    : TcpSocket()
{
    initiateConnection(host, port);
}

void TcpClientSocket::initiateConnection(
    const std::string &host, unsigned short port)
{
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);

    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) < 0) {
        throw socket_error("inet_pton() call failed");
    }

    if (::connect(m_socketfd, reinterpret_cast<sockaddr *>(&addr),
            sizeof(addr)) < 0) {
        throw socket_error("connect() call failed");
    }
}