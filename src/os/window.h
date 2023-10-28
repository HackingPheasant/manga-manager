#ifndef UI_WINDOW_H
#define UI_WINDOW_H

class Window {
public:
    struct Extent {
        std::uint32_t width;
        std::uint32_t height;
        Extent(std::uint32_t initial_width, std::uint32_t initial_height) : width(initial_width),
                                                        height(initial_height) {}
    } extent;
    std::string title;
    Window(std::string windowName, std::uint32_t width, std::uint32_t height) : extent(width, height), title(std::move(windowName)) {;
    virtual ~Window()=0;
    virtual createWindow()=0;
    virtual void getCurrentWindowSize()=0;
};

#endif //UI_WINDOW_H
