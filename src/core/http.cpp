#include <cpr/cpr.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "http.h"

namespace http {
auto get(std::string url) -> cpr::Response {
    // TODO Add cookies
    cpr::Response response = cpr::Get(cpr::Url{std::move(url)},
        cpr::Header{{"content-type", "application/json"}});

    //    // Handle Errors
    //    if (response.error) {
    //        throw std::runtime_error(fmt::format("Unable to download info\n Error : {}", response.error.message));
    //        throw std::runtime_error("Unable to download info\n Error : " + std::string(response.error.message));
    //    }
    //
    //    if (response.status_code != 200) {
    //        // TODO Do something better then this?
    //        throw std::runtime_error(fmt::format("Unable to download info\n Status code : {}", response.status_code));
    //    }

    return response;
}

auto post(std::string url) -> cpr::Response {
    // TODO Add cookies
    cpr::Response response = cpr::Post(cpr::Url{std::move(url)},
        cpr::Header{{"content-type", "application/json"}});

    return response;
}

} // namespace http
