#include <fstream>
#include <iostream>

#include <fmt/core.h>

#include "providers/mangadex.h"

Manga::Manga(const nlohmann::json &json) {
    // Reserve space in vector capacity to be at least enough to initially
    // contain "n" amount of elements
    tags.reserve(8);
    artists.reserve(2);
    authors.reserve(2);
    related_mangas.reserve(5);

    // Initalize the data in the class
    // Note: Below layout closely mimics the json structure
    // TODO: Handle null values in below areas
    id = json["data"]["id"].get<long>();                                             //id
    title = json["data"]["title"].get<std::string>();                                //title
    alt_titles = json["data"]["altTitles"].get<std::vector<std::string>>();          //alt_titles
    description = json["data"]["description"].get<std::string>();                    //description
    artists = json["data"]["artist"].get<std::vector<std::string>>();                //artists
    authors = json["data"]["author"].get<std::vector<std::string>>();                //authors
    original_lang_flag = json["data"]["publication"]["language"].get<std::string>(); //original_lang_flag
    publication_status = json["data"]["publication"]["status"].get<short>();         //publication_status
    demographic = json["data"]["publication"]["demographic"].get<short>();           //demographic
    tags = json["data"]["tags"].get<std::vector<short>>();                           //tags
    if (!(json["data"]["lastChapter"].empty())) {
        last_chapter = json["data"]["lastChapter"].get<std::string>(); //last_chapter
    }
    if (!(json["data"]["lastVolume"].empty())) {
        last_volume = json["data"]["lastVolume"].get<std::string>(); //last_volume
    }
    is_hentai = json["data"]["isHentai"]; //is_hentai
    if (!(json["data"]["links"].empty())) {
        links = json["data"]["links"].get<std::map<std::string, std::string>>(); //links
    }
    for (auto &rel : json["data"]["relations"].items()) { //related_mangas
        RelatedManga related;

        related.id = rel.value()["id"].get<long>();              //id
        related.title = rel.value()["title"].get<std::string>(); //title
        related.type = rel.value()["type"].get<short>();         //type
        related.is_hentai = rel.value()["isHentai"].get<bool>(); //is_hentai

        //Push related mangas into a vector
        related_mangas.push_back(related);
    }
    last_uploaded = json["data"]["lastUploaded"].get<long>();  //last_uploaded
    main_cover = json["data"]["mainCover"].get<std::string>(); //main_cover
}

void Manga::fetchAllCovers(const nlohmann::json &json) {
    for (auto &cover_info : json["data"].items()) { //covers
        Covers cover;

        cover.volume = cover_info.value()["volume"].get<std::string>(); //volume
        cover.url = cover_info.value()["url"].get<std::string>();       //url

        // Push cover object into a list
        covers.push_back(cover);
    }
}

void Manga::fetchChapterListing(const nlohmann::json &json) {
    for (auto &chap : json["data"]["chapters"].items()) { //partial_chapters
        PartialChapter chapter;

        chapter.chapter_id = chap.value()["id"].get<long>();                        //chapter_id
        chapter.chapter_hash = chap.value()["hash"].get<std::string>();             //chapter_hash
        chapter.manga_id = chap.value()["mangaId"].get<long>();                     //manga_id
        chapter.manga_title = chap.value()["mangaTitle"].get<std::string>();        //manga_title
        chapter.volume = chap.value()["volume"].get<std::string>();                 //volume
        chapter.chapter = chap.value()["chapter"].get<std::string>();               //chapter
        chapter.chapter_title = chap.value()["title"].get<std::string>();           //chapter_title
        chapter.translated_lang_flag = chap.value()["language"].get<std::string>(); //translated_lang_flag
        chapter.groups = chap.value()["groups"].get<std::vector<int>>();            //groups
        chapter.uploader = chap.value()["uploader"].get<long>();                    //uploader
        chapter.timestamp = chap.value()["timestamp"].get<long>();                  //timestamp
        //TODO Use uploader name if group == "unknown" (group: 2) or "no group" (group: 657)
        for (auto &group_info : json["data"]["groups"].items()) {
            chapter.groups_reference.try_emplace(group_info.value()["id"].get<long>(), group_info.value()["name"].get<std::string>());
        }

        // Push chapter object into a list
        partial_chapters.push_back(chapter);
    }
}

void Manga::fetchChapter(Chapter &chapter, const nlohmann::json &json) {
    chapter.chapter_id = json["data"]["id"].get<long>();                        //chapter_id
    chapter.chapter_hash = json["data"]["hash"].get<std::string>();             //chapter_hash
    chapter.manga_id = json["data"]["mangaId"].get<long>();                     //manga_id
    chapter.manga_title = json["data"]["mangaTitle"].get<std::string>();        //manga_title
    chapter.volume = json["data"]["volume"].get<std::string>();                 //volume
    chapter.chapter = json["data"]["chapter"].get<std::string>();               //chapter
    chapter.chapter_title = json["data"]["title"].get<std::string>();           //chapter_title
    chapter.translated_lang_flag = json["data"]["language"].get<std::string>(); //translated_lang_flag
    for (auto &group_info : json["data"]["groups"].items()) {                   //groups_reference
        chapter.groups_reference.try_emplace(group_info.value()["id"].get<long>(), group_info.value()["name"].get<std::string>());
    }
    chapter.uploader = json["data"]["uploader"].get<long>();               //uploader
    chapter.timestamp = json["data"]["timestamp"].get<long>();             //timestamp
    chapter.chapter_status = json["data"]["status"].get<std::string>();    //chapter_status
    chapter.pages = json["data"]["pages"].get<std::vector<std::string>>(); //pages
    chapter.server_url = json["data"]["server"].get<std::string>();        //server_url
    if (json.contains("serverFallback")) {
        chapter.server_url_fallback = json["data"]["serverFallback"].get<std::string>(); //server_url_fallback
    }
    if (json.contains("groupWebsite")) {
        chapter.group_website = json["data"]["groupWebsite"].get<std::string>(); //group_website
    }
}

// TODO Remove (below and fmt include) once this project is progressed more
// This program is intended to be a library and something else should handle
// the display of info
void Manga::prettyPrint() {
    // Pretty print a few things for testing
    fmt::print("ID: {}\n", id);
    fmt::print("Title: {}\n", title);
    fmt::print("Alternative titles(s):\n");
    for (const auto &i : alt_titles) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Description: {}\n", description);
    fmt::print("Artists:\n");
    for (const auto &i : artists) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Authors:\n");
    for (const auto &i : authors) {
        fmt::print("\t{}\n", i);
    }
    fmt::print("Original Language: {}\n", language_code_names.find(original_lang_flag)->second);
    fmt::print("Publishing Status: {}\n", publication_status_strings.find(publication_status)->second);
    if (demographic != 0) {
        fmt::print("Demograpic: {}\n", demographic_strings.find(demographic)->second);
    }
    fmt::print("Tags:\n");
    for (const auto &i : tags) {
        fmt::print("\t{}\n", tags_strings.find(i)->second);
    }
    fmt::print("Last Chapter: {}\n", last_chapter);
    fmt::print("Last Volume: {}\n", last_volume);
    fmt::print("Is Hentai?: {}\n", is_hentai);
    fmt::print("Links:\n");
    for (auto &[key, value] : links) {
        fmt::print("\t{} -> {}\n", key, value);
    }
    fmt::print("Related Manga:\n");
    for (const auto &i : related_mangas) {
        fmt::print("\tID: {}\n", i.id);
        fmt::print("\t\tTitle: {} ({})\n", i.title, relation_type_strings.find(i.type)->second);
        fmt::print("\t\tIs Hentai?: {}\n", i.is_hentai);
    }
    // TODO once https://en.cppreference.com/w/cpp/chrono/sys_info is
    // implemented convert UNIX Timestampt to readable time with std functions
    fmt::print("Timestamp (latest upload): {}\n", last_uploaded);
    fmt::print("Main Cover: {}\n", main_cover);
    fmt::print("Covers:\n");
    for (const auto &i : covers) {
        fmt::print("\tVolume {}: {}\n", i.volume, i.url);
    }

    fmt::print("Chapters:\n");
    for (const auto &i : partial_chapters) {
        fmt::print("\tChapter ID: {}\n", i.chapter_id);
        fmt::print("\t\tHash: {}\n", i.chapter_hash);
        fmt::print("\t\tVolume: {}\n", i.volume);
        fmt::print("\t\tChapter: {}\n", i.chapter);
        fmt::print("\t\tTitle: {}\n", i.chapter_title);
        fmt::print("\t\tTranslated Language: {}\n", language_code_names.find(i.translated_lang_flag)->second);
        fmt::print("\t\tGroup(s):\n");
        for (const auto &group_id : i.groups) {
            fmt::print("\t\t\t{} -> {}\n", group_id, i.groups_reference.find(group_id)->second);
        }
        fmt::print("\t\tUploader ID: {}\n", i.uploader);
        fmt::print("\t\tTimestamp: {}\n", i.timestamp);
    }
    fmt::print("Size of chapter list: {}\n", partial_chapters.size());
}
