#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <cstdint>
#include <string>

#include <vulkan/vulkan.hpp>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

// I wanted to name my class Window but I keep confliciting with X11 c code
// It was just easier to give in and rename the class then any other option
class AppWindow {
  public:
    struct Extent {
        int width;
        int height;

        Extent(int initial_width, int initial_height) : width(initial_width),
                                                        height(initial_height) {}
    } extent;

    const std::string title;

    SDL_Window *window;

    AppWindow(std::string, int, int);
    ~AppWindow();
    void getCurrentWindowSize();
};
#endif
