#pragma once

#include <string>

#include "tcpsocket.h"

class TcpClientSocket : public TcpSocket
{
public:
    TcpClientSocket(const std::string &host, unsigned short port);

private:
    void initiateConnection(const std::string &host, unsigned short port);
};
