#include <cpr/cpr.h>
#include <fmt/core.h>

namespace http {
auto get(std::string url) -> cpr::Response {
    // TODO Add cookies
    cpr::Response response = cpr::Get(cpr::Url{url});

    // Handle Errors
    if (response.error) {
        throw std::runtime_error(fmt::format("Unable to download info\n Error : {}", response.error.message));
    }

    if (response.status_code != 200) {
        // TODO Do something better then this?
        throw std::runtime_error(fmt::format("Unable to download info\n Status code : {}", response.status_code));
    }

    return response;
}

}
