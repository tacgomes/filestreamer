#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>


class socket_error : public std::runtime_error
{
public:
    socket_error(const std::string &what)
        : std::runtime_error(what + ": " + strerror(errno)) {};
};