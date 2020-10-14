#include <fstream>
#include <iostream>
#include <thread>

#include "filereceiver.h"
#include "tcpserversocket.h"

constexpr std::chrono::milliseconds FileReceiver::PollingTime;
constexpr uint32_t FileReceiver::MaxBytesNotAcknowledged;

FileReceiver::FileReceiver(unsigned port)
    : m_socket(new TcpServerSocket(port))
{
}

FileReceiver::~FileReceiver()
{
}

void FileReceiver::start()
{
    std::cout << "Listening for incoming connections\n";

    m_action = Action::Start;

    while (m_action == Action::Start) {
        std::unique_ptr<TcpSocket> socket = m_socket->accept();
        if (socket == nullptr) {
            std::this_thread::sleep_for(PollingTime);
        } else {
            process(std::move(socket));
        }
    }
}

void FileReceiver::stop()
{
    m_action = Action::Stop;
}

void FileReceiver::stopNow()
{
    m_action = Action::StopNow;
}

void FileReceiver::process(std::unique_ptr<TcpSocket> socket) const
{
    std::cout << "Processing new file upload request\n";

    uint8_t filenameLength;
    if (!socket->read(filenameLength)) {
        std::cerr << "ERROR: failed to read filename length\n";
        return;
    }

    TcpSocket::buffer_t filenameBytes;
    if (!socket->read(filenameBytes, filenameLength)) {
        std::cerr << "ERROR: failed to read filename\n";
        return;
    }

    uint32_t filesize;
    if (!socket->read(filesize)) {
        std::cerr << "ERROR: failed to read file size\n";
        return;
    }

    uint32_t offset;
    if (!socket->read(offset)) {
        std::cerr << "ERROR: failed to read file offset\n";
        return;
    }

    std::string filename(filenameBytes.begin(), filenameBytes.end());

    std::cout << "Parsed header: filename=" << filename
              << " filesize=" << filesize
              << " offset=" << offset << std::endl;

    std::ios::openmode openFlags = std::ios::binary | std::ios::out;
    if (offset > 0) {
        openFlags |= std::ios::in;
    }

    std::fstream outfile(filename + ".received", openFlags);
    if (!outfile.is_open()) {
        std::cerr << "ERROR: failed to open file " << filename + ".received"
                  << std::endl;
        return;
    }

    outfile.seekp(offset);

    uint32_t bytesReceived = 0;
    uint32_t bytesNotAcknowledged = 0;
    TcpSocket::buffer_t buffer;

    while (m_action != Action::StopNow) {
        if (!socket->read(buffer)) {
            std::cerr << "ERROR: file transfer failed\n";
            break;
        }

        if (buffer.size() == 0) {
            std::cout << "File transfer completed\n";
            break;
        }

        outfile.write(buffer.data(), buffer.size());

        bytesReceived += buffer.size();
        bytesNotAcknowledged += buffer.size();

        if (bytesNotAcknowledged >= MaxBytesNotAcknowledged
                || (offset + bytesReceived) == filesize) {
            outfile.flush();
            if (!socket->write(bytesReceived + offset)) {
                std::cerr << "WARNING: failed to send acknowledgement\n";
            }
            bytesNotAcknowledged = 0;
        }
    }
}
