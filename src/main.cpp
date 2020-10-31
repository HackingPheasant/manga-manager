#include <iostream>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "mangadex.h"

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " [options] <id>\n\n"
              << "Options:\n"
              << "\t-d,--download\t\tDownload Chapters\n"
              << "\t-o,--output-directory\tSpecify output directory.\n\t\t\t\tIf not specified then current directory is used\n"
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
    bool download = false; // Default to false and explicitly enable download
    std::string output_directory;
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

        if ((arg == "-d") || (arg == "--download")) {
            download = true;
        }

        if ((arg == "-o") || (arg == "--output-directory")) {
            output_directory = argv[i+1]; // I AM STUPID AND THIS IS BROKEN
        } else {
            output_directory = ".";
        }

        if (i + 1 < argc) {
            allArgs.push_back(argv[i]); // Add all but the last argument to the vector.
        } else {
            manga_id = argv[i];
        }
    }

    // Contruct API url and get JSON response
    std::string manga_api_url = MANGA_API + manga_id;
    cpr::Response response = cpr::Get(cpr::Url{manga_api_url});

    // Create an empty structure (null)
    auto json = nlohmann::json::parse(response.text);

    Manga manga(manga_id, json);

    manga.prettyPrint();
    std::string lang_code = "gb";
    manga.getDataChapters(lang_code);
    if (download) {
        manga.downloadChapters(output_directory);
    }
    return 0;
}
