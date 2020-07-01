#pragma once

#include <memory>

#include "tcpsocket.h"

class TcpServerSocket : public TcpSocket
{
public:
    explicit TcpServerSocket(unsigned short port);
    std::unique_ptr<TcpSocket> accept() const;

private:
    void bind(unsigned short port) const;
    void listen() const;

    constexpr static unsigned QueueLength = 5;
};
