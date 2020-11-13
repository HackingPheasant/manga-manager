#include <iostream>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <fmt/core.h> // Will change to std::format when compilers support it
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
    bool continue_downloading = true;
    std::string output_directory;
    // argv+1 to skip over executables location
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }

        if ((arg == "-V") || (arg == "--version")) {
            fmt::print("{} 0.1.0\n", argv[0]);
            return 0;
        }

        if ((arg == "-d") || (arg == "--download")) {
            download = true;
        }

        if ((arg == "-o") || (arg == "--output-directory")) {
            output_directory = argv[i + 1]; // I AM STUPID AND THIS IS BROKEN
        } else {
            output_directory = ".";
        }

        if (i + 1 < argc) {
            allArgs.push_back(argv[i]); // Add all but the last argument to the vector.
        } else {
            manga_id = argv[i];
        }
    }

    // Contruct API url for manga and get JSON response
    std::string manga_api_url = MANGA_API + manga_id;
    cpr::Response response = cpr::Get(cpr::Url{manga_api_url});
    auto json = nlohmann::json::parse(response.text);

    // Handle Errors
    if (response.error) {
        throw std::runtime_error(fmt::format("Unable to download info for manga \"{}\"\n Error : {}", manga_id, response.error.message));
    }

    if (response.status_code != 200) {
        throw std::runtime_error(fmt::format("Unable to download info for manga \"{}\"\n Status code : {}", manga_id, response.status_code));
    }

    Manga manga(manga_id, json);

    manga.prettyPrint();

    // Download chapters if passed download flag
    while (download && continue_downloading) {
        std::string chapter_id;
        fmt::print("Enter chapter ID to download specific chapter: ");
        std::cin >> chapter_id;

        // Contruct API url for chapter and get JSON response
        std::string chapter_api_url = CHAPTER_API + chapter_id;
        response = cpr::Get(cpr::Url{chapter_api_url});
        json = nlohmann::json::parse(response.text);

        // Handle Errors
        // TODO: Maybe make this into a function to remove duplication?
        if (response.error) {
            throw std::runtime_error(fmt::format("Unable to download info for chapter \"{}\"\n Error : {}", chapter_id, response.error.message));
        }

        if (response.status_code != 200) {
            throw std::runtime_error(fmt::format("Unable to download info for chapter \"{}\"\n Status code : {}", chapter_id, response.status_code));
        }

        // Nested Objected (Chapter) created, then the data will be filled in with the fetchChapter function
        Manga::Chapter chapter;
        manga.fetchChapter(chapter, json);
        manga.downloadChapter(chapter, output_directory);

        // Ask user if they want to download more chapters
        std::string input;
        fmt::print("Download more chapters? (y/n)\n");
        std::cin >> input;

        if (input == "y") {
            continue_downloading = true;
        } else {
            continue_downloading = false;
        }
    }
    return 0;
}
