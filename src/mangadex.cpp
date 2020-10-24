#include <fstream>
#include <iostream>
#include <list>
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
        auto split_str_trimmed{ trim(split_str) };
        vec.push_back(std::string{split_str_trimmed});
    }
}

bool boolean_convert (short b) {
    return b ? true : false;
}

void Manga::prettyPrint() {
    // Pretty print a few things for testing
    std::cout << "ID: " << id << std::endl;
    std::cout << "Title: " << title << std::endl;
    std::cout << "Description: " << description << std::endl;
    std::cout << "Is Hentai?: " << std::boolalpha << is_hentai<< std::endl;
    std::cout << "Publishing Status: " << pub_status_strings.find(pub_status)->second << std::endl;
    std::cout << "Original Language: " << orig_lang_name << " (" << orig_lang_flag << ")" << std::endl;
    std::cout << "Main Cover: " << BASE_URL + cover_url << std::endl;
    std::cout << "Links: " << std::endl;
    for(auto& [key, value] : links) {
        std::cout << "\t" << key << " -> " << value << std::endl;
    }
    if (demographic != 0) {
        std::cout << "Demograpic: " << demographic_strings.find(demographic)->second << std::endl;
    }
    std::cout << "Genres: " << std::endl;
    for(auto i : genres) {
        std::cout << "\t" << genre_strings.find(i)->second << std::endl;
    }
    std::cout << "Alternative name(s): " << std::endl;
    for(auto i : alt_names) {
        std::cout << "\t" << i << std::endl;
    }
    std::cout << "Artists: " << std::endl;
    for(auto i : artists) {
        std::cout << "\t" << i << std::endl;
    }
    std::cout << "Authors: " << std::endl;
    for(auto i : authors) {
        std::cout << "\t" << i << std::endl;
    }
    std::cout << "Covers: " << std::endl;
    for(auto i : covers) {
        std::cout << "\t" << BASE_URL + i << std::endl;
    }
    std::cout << "Related Manga: " << std::endl;
    for(auto i : related_mangas) {
        std::cout << "\tID: " << i.related_manga_id << std::endl;
        std::cout << "\t\tTitle: " << i.manga_name << " ("<< related_id_strings.find(i.relation_id)->second << ")" << std::endl;
        std::cout << "\t\tIs Hentai?: " << i.is_related_manga_hentai << std::endl;
    }
    std::cout << "Chapters (Partial Info): " << std::endl;
    for(auto i : partial_chapters) {
        std::cout << "\tChapter ID: " << i.id << std::endl;
        std::cout << "\t\tTimestamp: " << i.timestamp << std::endl;
        std::cout << "\t\tVolume: " << i.volume << std::endl;
        std::cout << "\t\tChapter: " << i.chapter << std::endl;
        std::cout << "\t\tTitle: " << i.title << std::endl;
        std::cout << "\t\tTranslated Language: " << i.lang_name << " (" << i.lang_code << ")" << std::endl;
        std::cout << "\t\tGroup(s): " << std::endl;
        for(auto& [key, value] : i.groups) {
            std::cout << "\t\t\t" << key << " -> " << value << std::endl;
        }
    }
    std::cout << "Size of list: " << partial_chapters.size() << std::endl;
}

Manga::Manga(std::string manga_id) {
    using json = nlohmann::json;
    
    // Contruct API url and get JSON response
    std::string manga_api_url = MANGA_API + manga_id;
    cpr::Response r = cpr::Get(cpr::Url{manga_api_url});

    // Create an empty structure (null)
    auto j = json::parse(r.text);

    // Reserve space in vector capacity to be at least enough to contain "n" amount of elements
    genres.reserve(8);
    artists.reserve(2);
    authors.reserve(2);
    related_mangas.reserve(5);

    // Initalize the data in the class
    id = std::stoul(manga_id); //id
    cover_url = j["manga"]["cover_url"]; //cover_url
    description = j["manga"]["description"]; //description
    is_hentai = boolean_convert(j["manga"]["hentai"]); //is_hentai
    pub_status = j["manga"]["status"]; //pub_status
    demographic = j["manga"]["demographic"]; //demographic
    title = j["manga"]["title"]; // title 
    orig_lang_name = j["manga"]["lang_name"]; //orig_lang_name
    orig_lang_flag = j["manga"]["lang_flag"]; //orig_lang_flag
    genres = j["manga"]["genres"].get<std::vector<short>>(); //genres
    alt_names = j["manga"]["alt_names"].get<std::vector<std::string>>(); //alt_names
    // TODO: Handle null values in below areas
    std::string artists_string = j["manga"]["artist"];
    std::string authors_string = j["manga"]["author"];
    split_string_into_vector(artists_string, ',', artists); //artists
    split_string_into_vector(authors_string, ',', authors); //authors
    covers = j["manga"]["covers"].get<std::list<std::string>>(); //covers
    links = j["manga"]["links"].get<std::map<std::string, std::string>>(); //links

    // related_mangas
    for (auto &rel : j["manga"]["related"].items())  {
        RelatedManga related;

        related.relation_id = rel.value()["relation_id"].get<short>();
        related.related_manga_id = rel.value()["related_manga_id"].get<int>();
        related.manga_name = rel.value()["manga_name"].get<std::string>();
        related.is_related_manga_hentai = rel.value()["manga_hentai"].get<short>();

        //Push related mangas into a vector
        related_mangas.push_back(related);
    }

    //partial_chapters
    for (auto &chap : j["chapter"].items()) {
        PartialChapter chapter;

        chapter.timestamp = chap.value()["timestamp"].get<unsigned long>();
        chapter.id = chap.key();
        chapter.volume = chap.value()["volume"].get<std::string>();
        chapter.chapter = chap.value()["chapter"].get<std::string>();
        chapter.title = chap.value()["title"].get<std::string>();
        chapter.lang_name = chap.value()["lang_name"].get<std::string>();
        chapter.lang_code = chap.value()["lang_code"].get<std::string>();
        chapter.groups.insert ( std::pair<int, std::string>(chap.value()["group_id"].get<int>(), chap.value()["group_name"].get<std::string>()) );
        if (chap.value()["group_id_2"].get<int>() != 0) {
            chapter.groups.insert ( std::pair<int, std::string>(chap.value()["group_id_2"].get<int>(), chap.value()["group_name_2"].get<std::string>()) );
        }
        if (chap.value()["group_id_3"].get<int>() != 0) {
            chapter.groups.insert ( std::pair<int, std::string>(chap.value()["group_id_3"].get<int>(), chap.value()["group_name_3"].get<std::string>()) );
        }
 
        // Push chapter object into a list
        partial_chapters.push_back(chapter);
    }

    // Download Cover
    // TODO: Move elsewhere so class initialization is quicker
    //r = cpr::Get(cpr::Url{cover_url});
    //writeFile(r.text, "Cover.jpeg");

}

void Manga::getDataChapters(std::string lang_code) {
    using json = nlohmann::json;

    // Is this the most efficent way? Have no clue
    // Why not just extend/add onto the object you already created? dunno how to do it properly
    // So instead I just use the data from the partial chapter objects so I can
    // get the data from the chapter API only for the chapters I want, then add all that
    // data into a new object and deleting the old object (dunno how to add or extend the current objects)
    for(auto i : partial_chapters) {
        if (i.lang_code == lang_code) {
            // Contruct API url and get JSON response
            std::string chapter_api_url = CHAPTER_API + i.id;
            std::cout << chapter_api_url << std::endl;
            cpr::Response r = cpr::Get(cpr::Url{chapter_api_url});

            // Create an empty structure (null)
            auto j = json::parse(r.text);

            //partial_chapters
            Chapter chapter;

            chapter.id = j["id"].get<unsigned long>();
            chapter.timestamp = j["timestamp"].get<unsigned long>();
            chapter.long_strip = j["long_strip"].get<bool>();
            chapter.chapter_status = j["status"].get<std::string>();
            chapter.volume = j["volume"].get<std::string>();
            chapter.chapter = j["chapter"].get<std::string>();
            chapter.title = j["title"].get<std::string>();
            chapter.lang_name = j["lang_name"].get<std::string>();
            chapter.lang_code = j["lang_code"].get<std::string>();
            chapter.groups.insert ( std::pair<int, std::string>(j["group_id"].get<int>(), j["group_name"].get<std::string>()) );
            if (j["group_id_2"].get<int>() != 0) {
                chapter.groups.insert ( std::pair<int, std::string>(j["group_id_2"].get<int>(), j["group_name_2"].get<std::string>()) );
            }
            if (j["group_id_3"].get<int>() != 0) {
                chapter.groups.insert ( std::pair<int, std::string>(j["group_id_3"].get<int>(), j["group_name_3"].get<std::string>()) );
            }
            chapter.manga_hash = j["hash"].get<std::string>();
            chapter.server_url = j["server"].get<std::string>();
            chapter.server_url_fallback = j["server_fallback"].get<std::string>();
            chapter.page_array = j["page_array"].get<std::list<std::string>>();

            // Push chapter object into a list
            // Remove old objects from partial_chapter list
            // TODO Figure out how to remove the old partial chapter from the list 
            // since we already have it in the chapter list now
            // TODO Do We NEED this? 
            chapters.push_back(chapter);

            // remove old objects from partial_chapter list
            // todo figure out how to remove the old partial chapter from the list 
            // since we already have it in the chapter list now
        }
    }
}

bool Manga::downloadChapters(std::string output_directory) {
    std::cout << "Output Directory: " << output_directory << std::endl;

    for (auto i : chapters) {
        for (auto j : i.page_array) {
            std::string page_url = i.server_url + i.manga_hash + "/" + j;
            cpr::Response r = cpr::Get(cpr::Url{page_url});
            std::string out = output_directory+ "/" + j;
            writeFile(r.text, out);
        };
    };

    return 0;
}
