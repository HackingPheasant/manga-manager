#ifndef SRC_MANGADEX_H
#define SRC_MANGADEX_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>


const std::string BASE_URL = "https://mangadex.org";
const std::string MANGA_API = BASE_URL + "/api/manga/";
const std::string CHAPTER_API = BASE_URL + "/api/chapter/";

// Based off https://github.com/md-y/mangadex-full-api
/* Notes for reference
ChapterType != "OK" then {
    // Hosted and readable on MangaDex
    0 = "Internal",
    // Hosted but not yet readable on MangaDex
    1 = "Delayed",
    // Not hosted on MangaDex
    2 = "External"
}
*/

class Manga {
private:
    struct RelatedManga {
        short is_related_manga_hentai;
        short relation_id;
        int related_manga_id;
        std::string manga_name;
    };
    struct PartialChapter {
        unsigned long timestamp; // Year 2038 problem avoided (for now)
        std::string id;
        std::string volume;
        std::string chapter;
        std::string title;
        std::string lang_name;
        std::string lang_code;
        std::map<int, std::string> groups;
    };
    struct Chapter: PartialChapter {
        bool long_strip;
        unsigned long id;
        std::string chapter_status;
        std::string manga_hash;
        std::string server_url;
        std::string server_url_fallback;
        std::list<std::string> page_array;
    };
    bool is_hentai; // Should see about converting to bool
    short pub_status;
    short demographic;
    unsigned long id;
    unsigned long last_updated;
    std::string title;
    std::string description;
    std::string cover_url;
    std::string orig_lang_name;
    std::string orig_lang_flag;
    std::list<std::string> covers;
    std::list<PartialChapter> partial_chapters;
    std::list<Chapter> chapters;
    std::vector<short> genres;
    std::vector<std::string> alt_names;
    std::vector<std::string> artists;
    std::vector<std::string> authors;
    std::vector<RelatedManga> related_mangas;
    std::map<std::string, std::string> links;
    // Translate int->string
    const std::map<int, std::string> genre_strings = {
        { 1, "4-Koma" }, { 2, "Action" }, { 3, "Adventure" }, { 4, "Award Winning" }, { 5, "Comedy" },
        { 6, "Cooking" }, { 7, "Doujinshi" }, { 8, "Drama" }, { 9, "Ecchi" }, { 10, "Fantasy" },
        { 11, "Gyaru" }, { 12, "Harem" }, { 13, "Historical" }, { 14, "Horror" }, { 16, "Martial Arts" },
        { 17, "Mecha" }, { 18, "Medical" }, { 19, "Music" }, { 20, "Mystery" }, { 21, "Oneshot" },
        { 22, "Psychological" }, { 23, "Romance" }, { 24, "School Life" }, { 25, "Sci-Fi" }, { 28, "Shoujo Ai" },
        { 30, "Shounen Ai" }, { 31, "Slice of Life" }, { 32, "Smut" }, { 33, "Sports" }, { 34, "Supernatural" },
        { 35, "Tragedy" }, { 36, "Long Strip" }, { 37, "Yaoi" }, { 38, "Yuri" }, { 40, "Video Games" },
        { 41, "Isekai" }, { 42, "Adaptation" }, { 43, "Anthology" }, { 44, "Web Comic" }, { 45, "Full Color" },
        { 46, "User Created" }, { 47, "Official Colored" }, { 48, "Fan Colored" }, { 49, "Gore" },
        { 50, "Sexual Violence" }, { 51, "Crime" }, { 52, "Magical Girls" }, { 53, "Philosophical" },
        { 54, "Superhero" }, { 55, "Thriller" }, { 56, "Wuxia" }, { 57, "Aliens" }, { 58, "Animals" },
        { 59, "Crossdressing" }, { 60, "Demons" }, { 61, "Delinquents" }, { 62, "Genderswap" }, { 63, "Ghosts" },
        { 64, "Monster Girls" }, { 65, "Loli" }, { 66, "Magic" }, { 67, "Military" }, { 68, "Monsters" },
        { 69, "Ninja" }, { 70, "Office Workers" },{ 71, "Police" }, { 72, "Post-Apocalyptic" }, { 73, "Reincarnation" },
        { 74, "Reverse Harem" }, { 75, "Samurai" }, { 76, "Shota" }, { 77, "Survival" }, { 78, "Time Travel" },
        { 79, "Vampires" }, { 80, "Traditional Games" }, { 81, "Virtual Reality" }, { 82, "Zombies" }, { 83, "Incest" },
        { 84, "Mafia" }
    };
    const std::map<int, std::string> demographic_strings = {
        { 1, "Shounen" }, { 2, "Shoujo" }, { 3, "Seinen" }, {4, "Josei" }
    };
    const std::map<int, std::string> pub_status_strings = {
        { 1, "Ongoing" },{ 2, "Completed" }, {3, "Cancelled" },{ 4, "Hiatus" }
    };
    const std::map<int, std::string> related_id_strings = {
        { 1, "Prequel" }, {2, "Sequel" }, {3, "Adapted from" }, {4, "Spin-off" }, { 5, "Side story" },
        { 6, "Main story" }, { 7, "Alternate story" }, { 8, "Doujinshi" }, { 9, "Based on" }, { 10, "Coloured" },
        { 11, "Monochrome" }, { 12, "Shared universe" }, { 13, "Same franchise" }, { 14, "Pre-serialization" },
        { 15, "Serialization" }
    };
public:
    Manga(std::string, const nlohmann::json &); // Constructor
    void prettyPrint();
    void getDataChapters(std::string);
    bool downloadChapters(std::string);
};

#endif // SRC_MANGADEX_H
