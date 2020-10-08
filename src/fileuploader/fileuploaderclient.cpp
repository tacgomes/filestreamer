#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "fileuploader.h"

static void showUsage(const std::string &progname)
{
    std::cerr << "Usage: " << progname << " [Options...] FILE\n\n"
              << "Options:\n"
              << "\t--host HOST\t\tSpecify the server IP address\n"
              << "\t--port HOST\t\tSpecify the server port\n"
              << "\t--limit-rate RATE\tLimit upload speed (bytes/second)\n"
              << "\t-h,--help\t\tShow usage\n";
}

int main(int argc, char *argv[])
{
    std::string host;
    std::string filename;
    unsigned short port = 0;
    unsigned limitRate = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg (argv[i]);
        if (arg == "--host") {
            ++i;
            if (i < argc) {
                host = argv[i];
            } else {
                showUsage(argv[0]);
                break;
            }
        } else if (arg == "--port") {
            ++i;
            if (i < argc) {
                auto val = std::stoul(argv[i]);
                if (val > 65535) {
                    std::cerr << "Error: port must be between 1 and 65535\n";
                    return EXIT_FAILURE;
                }
                port = static_cast<unsigned short>(val);
            } else {
                showUsage(argv[0]);
                break;
            }
        } else if (arg == "--limit-rate") {
            ++i;
            if (i < argc) {
                limitRate = std::stoul(argv[i]);
            } else {
                showUsage(argv[0]);
                break;
            }
        } else if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            filename = argv[i];
        }
    }

    if (host.empty() || port == 0 || filename.empty()) {
        std::cerr << "Error: missing arguments\n\n";
        showUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        FileUploader uploader(host, port, limitRate);
        uploader.upload(filename);
    } catch (const std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}