#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include "ratelimiter.h"

class TcpSocket
{
public:
    using buffer_t = std::vector<char>;

    explicit TcpSocket(int socketfd);
    virtual ~TcpSocket();
    TcpSocket(const TcpSocket &) = delete;
    TcpSocket &operator=(const TcpSocket &) = delete;

    void setUploadSpeedLimit(unsigned bytesPerSec);

    unsigned bytesAvailable() const;

    bool read(uint8_t &num) const;
    bool read(uint32_t &num) const;
    bool read(buffer_t &buffer, unsigned n = 0) const;

    bool write(uint8_t num);
    bool write(uint32_t num);
    bool write(const buffer_t &buffer);

protected:
    TcpSocket(bool nonblocking = false);
    void createSocket(bool nonblocking);
    bool send(const void *buf, size_t len);

    int m_socketfd = -1;
    RateLimiter m_uploadingSpeedLimiter;

private:
    bool readExactBytes(char *buffer, size_t size) const;

    template <typename T>
    bool readIntegralValue(T &num) const;

    constexpr static unsigned DefaultBufferSize = 1460;
};

template <class T>
bool TcpSocket::readIntegralValue(T &num) const
{
    static_assert(std::is_integral<T>::value, "Integral required");
    return readExactBytes(reinterpret_cast<char *>(&num), sizeof(num));
}