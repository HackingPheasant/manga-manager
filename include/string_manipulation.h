#ifndef INCLUDE_STRING_MANIPULATION_H
#define INCLUDE_STRING_MANIPULATION_H

#include <string>
#include <string_view>
#include <vector>

namespace strings {
auto ltrim(std::string_view) -> std::string_view; // Trim beginning spaces
auto rtrim(std::string_view) -> std::string_view; // Trim ending spaces
auto trim(std::string_view) -> std::string_view;  // Trim both beginning and ending spaces

void split_into_vector(const std::string &, const char &, std::vector<std::string> &);
} // namespace strings
#endif // INCLUDE_STRING_MANIPULATION_H
