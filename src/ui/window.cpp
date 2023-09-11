#include <exception>

#include "window.h"

AppWindow::AppWindow(std::string windowName, int width, int height) : extent(width, height),
                                                                      title(std::move(windowName)) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error("Unable to initialize SDL: " + std::string(SDL_GetError()));
    }

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        title.c_str(),                                                        // window title
        SDL_WINDOWPOS_CENTERED,                                               // initial x position
        SDL_WINDOWPOS_CENTERED,                                               // initial y position
        extent.width,                                                         // width, in pixels
        extent.height,                                                        // height, in pixels
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI); // flags

    // Check that the window was successfully created
    if (window == nullptr) {
        throw std::runtime_error("Could not create window: " + std::string(SDL_GetError()));
    }
}

AppWindow::~AppWindow() {
    // Clean up the SDL2 mess
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void AppWindow::getCurrentWindowSize() {
    // Get the size of the windows underlying drawable dimension
    // May differ to SDL_GetWindowSize() if rendering to a high-DPI
    // drawable. e,g, Apples "Retina" screen
    SDL_Vulkan_GetDrawableSize(window, &extent.width, &extent.height);
}
