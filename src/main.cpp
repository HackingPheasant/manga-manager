#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

// Can have ordered_json when bug fixes for it are released in 3.9.1
// using json = nlohmann::ordered_json;
using json = nlohmann::json;

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " [options] <id>\n\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-V,--version\t\tDisplay version information"
              << std::endl;
}

int main(int argc, char** argv) {
   
    // TODO: Fix this very crude commandline parsing
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }
    std::vector <std::string> allArgs;    
    std::string manga_id;
    // argv+1 to skip over executables location
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }
        
        if ((arg == "-V") || (arg == "--version")) {
            std::cout << argv[0] << " 0.1.0" << std::endl;
            return 0;
        }

        if (i + 1 < argc) {
            allArgs.push_back(argv[i]); // Add all but the last argument to the vector.
        } else {
            manga_id = argv[i];
        }
    }


    std::string manga_url = "https://mangadex.org/api/manga/" + manga_id;
        
    cpr::Response r = cpr::Get(cpr::Url{manga_url});

    // Create an empty structure (null)
    auto j = json::parse(r.text);
    std::cout << j.dump(4) << std::endl;
    std::string cover = j["manga"]["cover_url"];
    std::cout << cover << std::endl;
    
    std::string cover_url = "https://mangadex.org/" + cover;
    r = cpr::Get(cpr::Url{cover_url});
    std::ofstream outf{"Cover.jpeg", std::ios::binary };
    if (!outf){
        std::cerr << "Failed to write" << std::endl;
        return 1;
    }
    outf << r.text;
    //std::cout << r.text << std::endl;
    // std::cout << r.url << std::endl;
    // std::cout << r.status_code << std::endl;
    // std::cout << r.header["content-type"] << std::endl;
    //std::cout << r.text << std::endl;
    
    return 0;
}
