#include <iostream>
#include <string>
#include <vector>

// Fallback for limited <ranges> support
#ifdef __has_include
#if __has_include(<ranges>)
#include <ranges>
#else
#include <sstream>
#endif
#endif

#include "strings.h"

namespace strings {
// https://stackoverflow.com/a/63050738/4634499
// Trim beginning spaces
auto ltrim(std::string_view str) -> std::string_view {
    const auto pos(str.find_first_not_of(" \t"));
    str.remove_prefix(pos);
    return str;
}

// Trim ending spaces
auto rtrim(std::string_view str) -> std::string_view {
    const auto pos(str.find_last_not_of(" \t"));
    str.remove_suffix(str.length() - pos - 1);
    return str;
}

// Trim both beginning and ending spaces
auto trim(std::string_view str) -> std::string_view {
    str = ltrim(str);
    str = rtrim(str);
    return str;
}

auto split(const std::string &str, const char &delimiter) -> std::vector<std::string> {
    std::vector<std::string> vec;

    // <ranges> is currently (2021-01-04) only availble in gcc 10.x.x, so fallback
    // to using <sstream> method of spliting strings.
#if __cpp_lib_ranges
    for (auto split_str : str | std::ranges::views::split(delimiter) | std::ranges::views::transform([](auto &&sub) {
             return std::string_view(&*sub.begin(),
                 static_cast<std::string_view::size_type>(std::ranges::distance(sub)));
         })) {
        // Trim leading and trailing spaces and push into vector
        auto split_str_trimmed{trim(split_str)};
        vec.emplace_back(std::string{split_str_trimmed});
    }
#else
    std::string split_str;
    std::stringstream ss(str);
    while (ss.good()) {
        std::getline(ss, split_str, delimiter);
        auto split_str_trimmed{trim(split_str)};
        vec.emplace_back(std::string{split_str_trimmed});
    }
#endif
    return vec;
}
} // namespace strings
