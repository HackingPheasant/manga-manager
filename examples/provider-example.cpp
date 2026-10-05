// SPDX-FileCopyrightText: © 2023 HackingPheasant <HackingPheasant@protonmail.com>
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>
#include <string>

//#include <nlohmann/json.hpp>

//#include "http.h"
//#include "mangadex.h"

void showUsage(const std::string &name) {
    std::cerr << "Usage: " << name << " [options] <id>\n\n"
              << "Options:\n"
              << "\t-d,--download\t\tDownload Chapters\n"
              << "\t-o,--output-directory\tSpecify output directory.\n\t\t\t\tIf not specified then current directory is used\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-V,--version\t\tDisplay version information\n";
}

auto writeFile(const std::string &data, const std::string &filename) -> bool {
    std::ofstream outf{filename, std::ios::binary};
    if (!outf) {
        std::cerr << "Failed to write" << filename << "\n";
        return false;
    }
    outf << data;
    return true;
}

auto main(int argc, const char **argv) -> int {
    // TODO: Fix this very crude commandline parsing
    if (argc < 2) {
        showUsage(argv[0]); //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                             //Disable lint for now, as this will probably change later
        return 1;
    }

    // Unimplemented

    return 0;
}

