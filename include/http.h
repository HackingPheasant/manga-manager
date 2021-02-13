#ifndef INCLUDE_HTTP_H
#define INCLUDE_HTTP_H

#include <cpr/cpr.h>

namespace http {

// what the function needs at minimum: url
//  url, cookies (tbd)
auto get(std::string) -> cpr::Response;

} // namespace http

#endif // INCLUDE_HTTP_H
