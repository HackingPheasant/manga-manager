#include <fstream>
#include <iostream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "mangadex.h"

bool Manga::writeFile(std::string data, std::string filename) {
    std::ofstream outf{filename, std::ios::binary};
    if (!outf) {
        std::cerr << "Failed to write" << filename << std::endl;
        return 1;
    }
    outf << data;
    return 0;
}

/* bool downloadCover(std::string url) {
    
} */

// TODO BREAK OUT BELOW INTO FUNCTIONS

Manga::Manga(std::string manga_id) {
    using json = nlohmann::json;
    // Can have ordered_json when bug fixes for it are released in 3.9.1
    // using json = nlohmann::ordered_json;
    
    id = manga_id; //id
    std::string manga_url = MANGA_API + id;

    cpr::Response r = cpr::Get(cpr::Url{manga_url});

    // Create an empty structure (null)
    auto j = json::parse(r.text);
    std::cout << j.dump(4) << std::endl;

    // genres.reserve(8);
    // chapters.reserve(10);
    // artist.reserve(1);
    // author.reserve(1);
    std::string cover = j["manga"]["cover_url"];
    cover_url = BASE_URL + cover; //cover_url
    description = j["manga"]["description"]; //description
    is_hentai = j["manga"]["hentai"]; //is_hentai
    pub_status = j["manga"]["status"]; //pub_status
    title = j["manga"]["title"]; // title 
    orig_lang_name = j["manga"]["lang_name"]; //orig_lang_name
    orig_lang_flag = j["manga"]["lang_flag"]; //orig_lang_flag
    // genres = j["manga"]["genres"]; //genres
    // chapters = j["manga"]
    // artists = j["manga"]["artist"]; //artist
    // authors = j["manga"]["author"]; //author
    // links = j["manga"]["

    r = cpr::Get(cpr::Url{cover_url});
    writeFile(r.text, "Cover.jpeg");

}
