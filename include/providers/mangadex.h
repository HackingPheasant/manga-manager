#ifndef INCLUDE_MANGADEX_H
#define INCLUDE_MANGADEX_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

const std::string BASE_URL = "https://mangadex.org";
const std::string MANGA_API = BASE_URL + "/api/v2/manga/";
const std::string CHAPTER_API = BASE_URL + "/api/v2/chapter/";

// Based off https://github.com/md-y/mangadex-full-api

class Manga {
  private:
    struct Covers {
        std::string volume;
        std::string url;
    };
    struct RelatedManga {
        bool is_hentai;
        short type;
        long id;
        std::string title;
    };
    struct PartialChapter {
        long timestamp;
        long uploader;
        long chapter_id;
        long manga_id;
        std::string chapter_title;
        std::string manga_title;
        std::string volume;
        std::string chapter;
        std::string translated_lang_flag;
        std::string chapter_hash;
        std::vector<int> groups;
        std::map<long, std::string> groups_reference;
    };
    bool is_hentai;
    short publication_status;
    short demographic;
    long id;
    long last_uploaded;
    std::string title;
    std::string description;
    std::string main_cover;
    std::string original_lang_flag;
    std::string last_chapter;
    std::string last_volume;
    std::list<PartialChapter> partial_chapters;
    std::vector<short> tags;
    std::vector<std::string> alt_titles;
    std::vector<std::string> artists;
    std::vector<std::string> authors;
    std::vector<Covers> covers;
    std::vector<RelatedManga> related_mangas;
    std::map<std::string, std::string> links;
    // Translate int->string
    const std::map<int, std::string> tags_strings = {
        // Refernce below link to define what "tags" fall into which content groups. TODO
        // https://mangadex.org/api/v2/tag
        {1, "4-Koma"}, {2, "Action"}, {3, "Adventure"}, {4, "Award Winning"}, {5, "Comedy"},
        {6, "Cooking"}, {7, "Doujinshi"}, {8, "Drama"}, {9, "Ecchi"}, {10, "Fantasy"},
        {11, "Gyaru"}, {12, "Harem"}, {13, "Historical"}, {14, "Horror"}, {16, "Martial Arts"},
        {17, "Mecha"}, {18, "Medical"}, {19, "Music"}, {20, "Mystery"}, {21, "Oneshot"},
        {22, "Psychological"}, {23, "Romance"}, {24, "School Life"}, {25, "Sci-Fi"}, {28, "Shoujo Ai"},
        {30, "Shounen Ai"}, {31, "Slice of Life"}, {32, "Smut"}, {33, "Sports"}, {34, "Supernatural"},
        {35, "Tragedy"}, {36, "Long Strip"}, {37, "Yaoi"}, {38, "Yuri"}, {40, "Video Games"},
        {41, "Isekai"}, {42, "Adaptation"}, {43, "Anthology"}, {44, "Web Comic"}, {45, "Full Color"},
        {46, "User Created"}, {47, "Official Colored"}, {48, "Fan Colored"}, {49, "Gore"},
        {50, "Sexual Violence"}, {51, "Crime"}, {52, "Magical Girls"}, {53, "Philosophical"},
        {54, "Superhero"}, {55, "Thriller"}, {56, "Wuxia"}, {57, "Aliens"}, {58, "Animals"},
        {59, "Crossdressing"}, {60, "Demons"}, {61, "Delinquents"}, {62, "Genderswap"}, {63, "Ghosts"},
        {64, "Monster Girls"}, {65, "Loli"}, {66, "Magic"}, {67, "Military"}, {68, "Monsters"},
        {69, "Ninja"}, {70, "Office Workers"}, {71, "Police"}, {72, "Post-Apocalyptic"}, {73, "Reincarnation"},
        {74, "Reverse Harem"}, {75, "Samurai"}, {76, "Shota"}, {77, "Survival"}, {78, "Time Travel"},
        {79, "Vampires"}, {80, "Traditional Games"}, {81, "Virtual Reality"}, {82, "Zombies"}, {83, "Incest"},
        {84, "Mafia"}};
    const std::map<int, std::string> demographic_strings = {
        {1, "Shounen"}, {2, "Shoujo"}, {3, "Seinen"}, {4, "Josei"}};
    const std::map<int, std::string> publication_status_strings = {
        {1, "Ongoing"}, {2, "Completed"}, {3, "Cancelled"}, {4, "Hiatus"}};
    const std::map<int, std::string> relation_type_strings = {
        // Refernce below link to define what relations are "linked"
        // https://mangadex.org/api/v2/relations
        {1, "Prequel"}, {2, "Sequel"}, {3, "Adapted from"}, {4, "Spin-off"}, {5, "Side story"},
        {6, "Main story"}, {7, "Alternate story"}, {8, "Doujinshi"}, {9, "Based on"}, {10, "Coloured"},
        {11, "Monochrome"}, {12, "Shared universe"}, {13, "Same franchise"}, {14, "Pre-serialization"},
        {15, "Serialization"}};
    // Translate lang_flag -> lang_name
    const std::map<std::string, std::string> language_code_names = {
        {"sa", "Arabic"}, {"bd", "Bengali"}, {"bg", "Bulgarian"}, {"mm", "Burmese"}, {"ct", "Catalan"},
        {"cn", "Chinese (Simp)"}, {"hk", "Chinese (Trad)"}, {"cz", "Czech"}, {"dk", "Danish"}, {"nl", "Dutch"},
        {"gb", "English"}, {"ph", "Filipino"}, {"fi", "Finnish"}, {"fr", "French"}, {"de", "German"},
        {"gr", "Greek"}, {"hu", "Hungarian"}, {"id", "Indonesian"}, {"il", "Hebrew"}, {"in", "Hindi"},
        {"it", "Italian"}, {"jp", "Japanese"}, {"kr", "Korean"}, {"lt", "Lithuanian"}, {"my", "Malay"},
        {"mn", "Mongolian"}, {"ir", "Persian"}, {"pl", "Polish"}, {"br", "Portuguese (Br)"}, {"pt", "Portuguese (Pt)"},
        {"ro", "Romanian"}, {"ru", "Russian"}, {"rs", "Serbo-Croatian"}, {"es", "Spanish (Es)"}, {"mx", "Spanish (LATAM)"},
        {"se", "Swedish"}, {"th", "Thai"}, {"tr", "Turkish"}, {"ua", "Ukrainian"}, {"vn", "Vietnamese"}};

  public:
    struct Chapter : PartialChapter {
        /* Notes for reference
         * ChapterStatus != "OK" then {
         *  0 = "Internal" -> Hosted and readable on MangaDex
         *  1 = "Delayed" -> Hosted but not yet readable on MangaDex
         *  2 = "External" -> Not hosted on MangaDex
         * }
         */
        std::string chapter_status;
        std::string server_url;
        std::string server_url_fallback; // Not allways available in json
        std::string group_website;       // Only in json if chapter_status == delayed
        std::vector<std::string> pages;  // Pages stops being an array when chapter_status == external
        // id: 9097/name: MangaPlus, are the only one so far that gets the "exernal" status
        // TODO Handle the external data option e.g. https://mangadex.org/api/chapter/670701
    };
    Manga(const nlohmann::json &); // Constructor
    void fetchAllCovers(const nlohmann::json &);
    void fetchChapterListing(const nlohmann::json &);
    static void fetchChapter(Chapter &, const nlohmann::json &);
    void prettyPrint();
};

#endif // INCLUDE_MANGADEX_H
