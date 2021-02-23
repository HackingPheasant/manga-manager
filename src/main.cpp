#include <iostream>
#include <string>
#include <vector>

#include <fmt/core.h> // Will change to std::format when compilers support it
#include <nlohmann/json.hpp>

#include "http.h"
#include "providers/mangadex.h"

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

auto downloadChapter(const Manga::Chapter &chapter, const std::string &output_directory) -> bool {
    fmt::print("Downloading.\nOutput Directory: {}\n", output_directory);

    for (const auto &page : chapter.pages) {
        std::string page_url = chapter.server_url + chapter.chapter_hash + "/" + page;
        // TODO Make use of server fallback url
        auto response = http::get(page_url);
        std::string output = output_directory;
        output.append("/");
        output.append(page);
        writeFile(response.text, output);
    };
    return true;
}

auto main(int argc, const char **argv) -> int {
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

    // Construct the URLs for below usage
    std::string manga_api_url = MANGA_API + manga_id;
    std::string manga_covers_url = MANGA_API + manga_id + "/covers";
    std::string manga_chapters_url = MANGA_API + manga_id + "/chapters";

    // Filling in some objects
    // Manga
    auto response = http::get(manga_api_url);
    auto json = nlohmann::json::parse(response.text);
    Manga manga(json);

    // Covers
    response = http::get(manga_covers_url);
    json = nlohmann::json::parse(response.text);
    manga.fetchAllCovers(json);

    // Partial Chapters
    response = http::get(manga_chapters_url);
    json = nlohmann::json::parse(response.text);
    manga.fetchChapterListing(json);

    // Print it all to the terminal :)
    manga.prettyPrint();

    // Download chapters if passed download flag
    while (download && continue_downloading) {
        fmt::print("Enter chapter ID to download specific chapter: ");
        std::string chapter_id;
        std::cin >> chapter_id;

        // Contruct API url for chapter and get JSON response
        std::string chapter_api_url = CHAPTER_API + chapter_id;
        response = http::get(chapter_api_url);
        json = nlohmann::json::parse(response.text);

        // Nested Objected (Chapter) created, then the data will be filled in with the fetchChapter function
        Manga::Chapter chapter;
        manga.fetchChapter(chapter, json);
        downloadChapter(chapter, output_directory);

        // Ask user if they want to download more chapters
        fmt::print("Download more chapters? (y/n)\n");
        std::string input;
        std::cin >> input;

        if (input == "y") {
            continue_downloading = true;
        } else {
            continue_downloading = false;
        }
    }
    return 0;
}
