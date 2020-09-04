#include <iostream>
#include <string>
#include <vector>

#include "mangadex.h"

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " [options] <id>\n\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-V,--version\t\tDisplay version information"
              << std::endl;
}

int main(int argc, char **argv) {
    // TODO: Fix this very crude commandline parsing
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }
    std::vector<std::string> allArgs;
    std::string manga_id;
    // argv+1 to skip over executables location
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }

        if ((arg == "-V") || (arg == "--version")) {
            std::cout << argv[0] << " 0.1.0" << std::endl;
            return 0;
        }

        if (i + 1 < argc) {
            allArgs.push_back(argv[i]); // Add all but the last argument to the vector.
        } else {
            manga_id = argv[i];
        }
    }

    Manga manga(manga_id);

    return 0;
}
