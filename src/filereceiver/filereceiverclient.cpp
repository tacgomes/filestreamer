#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "filereceiver.h"

static void showUsage(const std::string &progname)
{
    std::cerr << "Usage: " << progname << " [Options...] PORT\n\n"
              << "Options:\n"
              << "\t-h,--help\tShow usage\n";
}

int main(int argc, char *argv[])
{
    unsigned short port = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg (argv[i]);
        if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            auto val = std::stoul(argv[1]);
            if (val == 0 || val > 65535) {
                std::cerr << "Error: port must be between 1 and 65535\n";
                return EXIT_FAILURE;
            }
            port = static_cast<unsigned short>(val);
        }
    }

    if (port == 0) {
        std::cerr << "Error: missing arguments\n\n";
        showUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        FileReceiver receiver(port);
        receiver.start();
    } catch (const std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}