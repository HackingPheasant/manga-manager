#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <ranges>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "mangadex.h"

namespace ranges = std::ranges;

bool writeFile(std::string data, std::string filename) {
    std::ofstream outf{filename, std::ios::binary};
    if (!outf) {
        std::cerr << "Failed to write" << filename << std::endl;
        return 1;
    }
    outf << data;
    return 0;
}

// https://stackoverflow.com/a/63050738/4634499
// Trim beginning spaces
std::string_view ltrim(std::string_view str) {
    const auto pos(str.find_first_not_of(" \t"));
    str.remove_prefix(pos);
    return str;
}

// Trim ending spaces
std::string_view rtrim(std::string_view str) {
    const auto pos(str.find_last_not_of(" \t"));
    str.remove_suffix(str.length() - pos - 1);
    return str;
}

// Trim both beginning and ending spaces
std::string_view trim(std::string_view str) {
    str = ltrim(str);
    str = rtrim(str);
    return str;
}

void split_string_into_vector (std::string str, char delimiter, std::vector<std::string> &vec) {
    for (auto split_str : str
            | ranges::views::split(delimiter)
            | ranges::views::transform([](auto&& sub) {
                return std::string_view(&*sub.begin(),
                        static_cast<std::string_view::size_type>(ranges::distance(sub)));
                }))
    {
        auto split_str_trimmed{ ltrim(split_str) };
        vec.push_back(std::string{split_str_trimmed});
    }
}

void Manga::prettyPrint() {
    // Pretty print a few things for testing
    std::cout << "Title: " << title << std::endl;
    std::cout << "Description: " << description << std::endl;
    std::cout << "Is Hentai?: "<< is_hentai<< std::endl;
    std::cout << "Publishing Status: " << pub_status << std::endl;
    std::cout << orig_lang_name << " (" << orig_lang_flag << ")" <<std::endl;
    std::cout << cover_url << std::endl;
    std::cout << "Links: " << std::endl;
    for(auto& [key, value] : links) {
        std::cout << key << " -> " << value << std::endl;
    }
    std::cout << "Genres: " << std::endl;
    for(auto i : genres) {
        std::cout << "\t" << i << std::endl;
    }
    std::cout << "Artists: " << std::endl;
    for(auto i : artists) {
        std::cout << "\t" <<i << std::endl;
    }
    std::cout << "Authors: " << std::endl;
    for(auto i : authors) {
        std::cout << "\t" << i << std::endl;
    }
}

Manga::Manga(std::string manga_id) {
    using json = nlohmann::json;
    
    id = manga_id; //id
    std::string manga_api_url = MANGA_API + id;

    cpr::Response r = cpr::Get(cpr::Url{manga_api_url});

    // Create an empty structure (null)
    auto j = json::parse(r.text);

    // Reserve space in vector capacity to be at least enoug to contain n elements
    genres.reserve(8);
    //chapters.reserve(10);
    artists.reserve(2);
    authors.reserve(2);
 
    std::string cover = j["manga"]["cover_url"];
    cover_url = BASE_URL + cover; //cover_url
    description = j["manga"]["description"]; //description
    is_hentai = j["manga"]["hentai"]; //is_hentai
    pub_status = j["manga"]["status"]; //pub_status
    title = j["manga"]["title"]; // title 
    orig_lang_name = j["manga"]["lang_name"]; //orig_lang_name
    orig_lang_flag = j["manga"]["lang_flag"]; //orig_lang_flag
    genres = j["manga"]["genres"].get<std::vector<int>>(); //genres
    //chapters = j["manga"]
    // TODO: Handle null values in below areas
    std::string artists_string = j["manga"]["artist"];
    std::string authors_string = j["manga"]["author"];
    split_string_into_vector(artists_string, ',', artists); //artists
    split_string_into_vector(authors_string, ',', authors); //authors
    links = j["manga"]["links"].get<std::map<std::string, std::string>>();
 
    // Download Cover
    // TODO: Move elsewhere so class initialization is quicker
    r = cpr::Get(cpr::Url{cover_url});
    writeFile(r.text, "Cover.jpeg");

}
