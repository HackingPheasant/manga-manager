#include <cstdint>
#include <string>
#include <string_view>

namespace window {
class Window {
  public:
    struct Extent {
        std::uint32_t width;
        std::uint32_t height;
    };

    struct Properties {
        std::string title;
        Extent extent = {1280, 720};
    };

    Window(std::string_view title);
    ~Window();
};
} // namespace window
