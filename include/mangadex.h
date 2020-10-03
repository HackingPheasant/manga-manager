#ifndef SRC_MANGADEX_H
#define SRC_MANGADEX_H

#include <map>
#include <string>
#include <vector>

// Based off https://github.com/md-y/mangadex-full-api
const std::string BASE_URL = "https://mangadex.org";
const std::string MANGA_API = BASE_URL + "/api/manga/";
const std::string CHPATER_API = BASE_URL + "/api/manga/";


/* const enum struct ChapterType {
    // Hosted and readable on MangaDex
    0 = "Internal",
    // Hosted but not yet readable on MangaDex
    1 = "Delayed",
    // Not hosted on MangaDex
    2 = "External"
}

const enum struct Demographic {
    1 = "Shounen",
    2 = "Shoujo",
    3 = "Seinen",
    4 = "Josei"
}

const enum struct Genre {
    1 = "4-Koma",
    2 = "Action",
    3 = "Adventure",
    4 = "Award Winning",
    5 = "Comedy",
    6 = "Cooking",
    7 = "Doujinshi",
    8 = "Drama",
    9 = "Ecchi",
    10 = "Fantasy",
    11 = "Gyaru",
    12 = "Harem",
    13 = "Historical",
    14 = "Horror",
    16 = "Martial Arts",
    17 = "Mecha",
    18 = "Medical",
    19 = "Music",
    20 = "Mystery",
    21 = "Oneshot",
    22 = "Psychological",
    23 = "Romance",
    24 = "School Life",
    25 = "Sci-Fi",
    28 = "Shoujo Ai",
    30 = "Shounen Ai",
    31 = "Slice of Life",
    32 = "Smut",
    33 = "Sports",
    34 = "Supernatural",
    35 = "Tragedy",
    36 = "Long Strip",
    37 = "Yaoi",
    38 = "Yuri",
    40 = "Video Games",
    41 = "Isekai",
    42 = "Adaptation",
    43 = "Anthology",
    44 = "Web Comic",
    45 = "Full Color",
    46 = "User Created",
    47 = "Official Colored",
    48 = "Fan Colored",
    49 = "Gore",
    50 = "Sexual Violence",
    51 = "Crime",
    52 = "Magical Girls",
    53 = "Philosophical",
    54 = "Superhero",
    55 = "Thriller",
    56 = "Wuxia",
    57 = "Aliens",
    58 = "Animals",
    59 = "Crossdressing",
    60 = "Demons",
    61 = "Delinquents",
    62 = "Genderswap",
    63 = "Ghosts",
    64 = "Monster Girls",
    65 = "Loli",
    66 = "Magic",
    67 = "Military",
    68 = "Monsters",
    69 = "Ninja",
    70 = "Office Workers",
    71 = "Police",
    72 = "Post-Apocalyptic",
    73 = "Reincarnation",
    74 = "Reverse Harem",
    75 = "Samurai",
    76 = "Shota",
    77 = "Survival",
    78 = "Time Travel",
    79 = "Vampires",
    80 = "Traditional Games",
    81 = "Virtual Reality",
    82 = "Zombies",
    83 = "Incest",
    84 = "Mafia"
}

const enum struct PubStatus {
    1 = "Ongoing",
    2 = "Completed",
    3 = "Cancelled",
    4 = "Hiatus"
} */

class Manga {
private:
    short is_hentai;
    short pub_status;
    std::string id;
    std::string title;
    std::string description;
    std::string cover_url;
    std::string orig_lang_name;
    std::string orig_lang_flag;
    std::vector<int> genres;
    // std::vector<int> chapters;
    std::vector<std::string> artists;
    std::vector<std::string> authors;
    std::map<std::string, std::string> links;
public:
    Manga(std::string); // Constructor
    void prettyPrint();
};

#endif // SRC_MANGADEX_H
