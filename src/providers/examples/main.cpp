#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <fmt/core.h> // Will change to std::format when compilers support it
#include <nlohmann/json.hpp>

#include "http.h"
#include "mangadex.h"

static void show_usage(const std::string &name) {
    std::cerr << "Usage: " << name << " [options] <id>\n\n"
              << "Options:\n"
              << "\t-d,--download\t\tDownload Chapters\n"
              << "\t-o,--output-directory\tSpecify output directory.\n\t\t\t\tIf not specified then current directory is used\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-V,--version\t\tDisplay version information"
              << std::endl;
}

auto writeFile(const std::string &data, const std::string &filename) -> bool {
    std::ofstream outf{filename, std::ios::binary};
    if (!outf) {
        std::cerr << "Failed to write" << filename << std::endl;
        return false;
    }
    outf << data;
    return true;
}

auto main(int argc, const char **argv) -> int {
    // TODO: Fix this very crude commandline parsing
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    // Unimplemented

    return 0;
}

