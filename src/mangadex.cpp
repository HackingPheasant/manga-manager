#include <fstream>
#include <iostream>
#include <ranges>
#include <string_view>

#include <cpr/cpr.h>
#include <fmt/core.h>

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

void split_string_into_vector(std::string str, char delimiter, std::vector<std::string> &vec) {
    for (auto split_str : str | ranges::views::split(delimiter) | ranges::views::transform([](auto &&sub) {
             return std::string_view(&*sub.begin(),
                 static_cast<std::string_view::size_type>(ranges::distance(sub)));
         })) {
        auto split_str_trimmed{trim(split_str)};
        vec.push_back(std::string{split_str_trimmed});
    }
}

bool boolean_convert(short b) {
    return b ? true : false;
}

void Manga::prettyPrint() {
    // Pretty print a few things for testing
    fmt::print("ID: {}\n", id);
    fmt::print("Title: {}\n", title);
    fmt::print("Description: {}\n", description);
    fmt::print("Is Hentai?: {}\n", is_hentai);
    fmt::print("Publishing Status: {}\n", pub_status_strings.find(pub_status)->second);
    fmt::print("Original Language: {} ({})\n", orig_lang_name, orig_lang_flag);
    fmt::print("Main Cover: {}\n", BASE_URL + cover_url);
    fmt::print("Links:\n");
    for (auto &[key, value] : links) {
        fmt::print("\t{} -> {}\n", key, value);
    }
    if (demographic != 0) {
        fmt::print("Demograpic: {}\n", demographic_strings.find(demographic)->second);
    }
    fmt::print("Genres:\n");
    for (auto i : genres) {
        fmt::print("\t{}\n", genre_strings.find(i)->second);
    }
    fmt::print("Alternative name(s):\n");
    for (auto i : alt_names) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Artists:\n");
    for (auto i : artists) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Authors:\n");
    for (auto i : authors) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Covers:\n");
    for (auto i : covers) {
        fmt::print("\t{}\n", BASE_URL + i);
    }
    fmt::print("Related Manga:\n");
    for (auto i : related_mangas) {
        fmt::print("\tID: {}\n", i.related_manga_id);
        fmt::print("\t\tTitle: {} ({})\n", i.manga_name, related_id_strings.find(i.relation_id)->second);
        fmt::print("\t\tIs Hentai?: {}\n", i.is_related_manga_hentai);
    }
    fmt::print("Chapters:\n");
    for (auto i : partial_chapters) {
        fmt::print("\tChapter ID: {}\n", i.id);
        fmt::print("\t\tTimestamp: {}\n", i.timestamp);
        fmt::print("\t\tVolume: {}\n", i.volume);
        fmt::print("\t\tChapter: {}\n", i.chapter);
        fmt::print("\t\tTitle: {}\n", i.title);
        fmt::print("\t\tTranslated Language: {} ({})\n", i.lang_name, i.lang_code);
        fmt::print("\t\tGroup(s):\n");
        for (auto &[key, value] : i.groups) {
            fmt::print("\t\t\t{} -> {}\n", key, value);
        }
    }
    fmt::print("Size of list: {}\n", partial_chapters.size());
}

Manga::Manga(std::string manga_id, const nlohmann::json &json) {
    // Reserve space in vector capacity to be at least enough to contain "n" amount of elements
    genres.reserve(8);
    artists.reserve(2);
    authors.reserve(2);
    related_mangas.reserve(5);

    // Initalize the data in the class
    id = std::stoul(manga_id);                                              //id
    cover_url = json["manga"]["cover_url"].get<std::string>();              //cover_url
    description = json["manga"]["description"].get<std::string>();          //description
    is_hentai = boolean_convert(json["manga"]["hentai"]);                   //is_hentai
    pub_status = json["manga"]["status"].get<short>();                      //pub_status
    demographic = json["manga"]["demographic"].get<short>();                //demographic
    title = json["manga"]["title"].get<std::string>();                      // title
    orig_lang_name = json["manga"]["lang_name"].get<std::string>();         //orig_lang_name
    orig_lang_flag = json["manga"]["lang_flag"].get<std::string>();         //orig_lang_flag
    genres = json["manga"]["genres"].get<std::vector<short>>();             //genres
    alt_names = json["manga"]["alt_names"].get<std::vector<std::string>>(); //alt_names
    // TODO: Handle null values in below areas
    std::string artists_string = json["manga"]["artist"].get<std::string>();
    std::string authors_string = json["manga"]["author"].get<std::string>();
    split_string_into_vector(artists_string, ',', artists);                   //artists
    split_string_into_vector(authors_string, ',', authors);                   //authors
    covers = json["manga"]["covers"].get<std::list<std::string>>();           //covers
    links = json["manga"]["links"].get<std::map<std::string, std::string>>(); //links

    // related_mangas
    for (auto &rel : json["manga"]["related"].items()) {
        RelatedManga related;

        related.relation_id = rel.value()["relation_id"].get<short>();
        related.related_manga_id = rel.value()["related_manga_id"].get<int>();
        related.manga_name = rel.value()["manga_name"].get<std::string>();
        related.is_related_manga_hentai = boolean_convert(rel.value()["manga_hentai"]);

        //Push related mangas into a vector
        related_mangas.push_back(related);
    }

    //partial_chapters
    for (auto &chap : json["chapter"].items()) {
        PartialChapter chapter;

        chapter.timestamp = chap.value()["timestamp"].get<unsigned long>();
        chapter.id = chap.key();
        chapter.volume = chap.value()["volume"].get<std::string>();
        chapter.chapter = chap.value()["chapter"].get<std::string>();
        chapter.title = chap.value()["title"].get<std::string>();
        chapter.lang_name = chap.value()["lang_name"].get<std::string>();
        chapter.lang_code = chap.value()["lang_code"].get<std::string>();
        chapter.groups.insert(std::pair<int, std::string>(chap.value()["group_id"].get<int>(), chap.value()["group_name"].get<std::string>()));
        if (chap.value()["group_id_2"].get<int>() != 0) {
            chapter.groups.insert(std::pair<int, std::string>(chap.value()["group_id_2"].get<int>(), chap.value()["group_name_2"].get<std::string>()));
        }
        if (chap.value()["group_id_3"].get<int>() != 0) {
            chapter.groups.insert(std::pair<int, std::string>(chap.value()["group_id_3"].get<int>(), chap.value()["group_name_3"].get<std::string>()));
        }

        // Push chapter object into a list
        partial_chapters.push_back(chapter);
    }
}

void Manga::fetchChapter(Chapter &chapter, const nlohmann::json &json) {
    chapter.id = json["id"].get<unsigned long>();
    chapter.timestamp = json["timestamp"].get<unsigned long>();
    chapter.long_strip = json["long_strip"].get<bool>();
    chapter.chapter_status = json["status"].get<std::string>();
    chapter.volume = json["volume"].get<std::string>();
    chapter.chapter = json["chapter"].get<std::string>();
    chapter.title = json["title"].get<std::string>();
    chapter.lang_name = json["lang_name"].get<std::string>();
    chapter.lang_code = json["lang_code"].get<std::string>();
    chapter.groups.insert(std::pair<int, std::string>(json["group_id"].get<int>(), json["group_name"].get<std::string>()));
    if (json["group_id_2"].get<int>() != 0) {
        chapter.groups.insert(std::pair<int, std::string>(json["group_id_2"].get<int>(), json["group_name_2"].get<std::string>()));
    }
    if (json["group_id_3"].get<int>() != 0) {
        chapter.groups.insert(std::pair<int, std::string>(json["group_id_3"].get<int>(), json["group_name_3"].get<std::string>()));
    }
    chapter.manga_hash = json["hash"].get<std::string>();
    chapter.server_url = json["server"].get<std::string>();
    // TODO Handle if fallback server isn't present in json response
    chapter.server_url_fallback = json["server_fallback"].get<std::string>();
    chapter.page_array = json["page_array"].get<std::list<std::string>>();
}

bool Manga::downloadChapter(const Chapter &chapter, std::string output_directory) {
    fmt::print("Downloading.\nOutput Directory: {}\n", output_directory);

    for (auto i : chapter.page_array) {
        std::string page_url = chapter.server_url + chapter.manga_hash + "/" + i;
        cpr::Response r = cpr::Get(cpr::Url{page_url});
        std::string out = output_directory + "/" + i;
        writeFile(r.text, out);
    };
    return 0;
}
