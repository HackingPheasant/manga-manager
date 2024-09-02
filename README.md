# manga-manager
Manga manger and downloader built with C++

# For Developers
## Build
1. Configure
    ```bash
    cmake --preset <name>
    ```
To list the available presets that can be used, run `--list-presets=all`.
Some available options include `dev`, `release`, `relwithdebinfo` etc.

To use a compiler different to the system default:
In the configure step, append the following (as an example):
   `-D CMAKE_C_COMPILER=gcc-10.2 -D CMAKE_CXX_COMPILER=g++-10.2`

2. Build
    ```bash
    cmake --build --preset <name>
    ```

If you use `cmake --build` instead of directly calling the underlying build system you can use:
- `--parallel N` or `-j N` for parallel builds on *N* amount of cores
- `--target` or `-t` to pick a target. E.g. `--target clean`
- `-v` for verbose builds

## Test
    ```bash
    ctest --preset <name>
    ```
## Install/Package

**Install**
    ```bash
    cmake --build --preset <name> --target install
    ```
- `--prefix /my/install/prefix` to change the install directory from default.
Note: The Dev preset outputs defaults to `out/` in top level folder.

**Package**
    ```bash
    # Package the built binaries
    cpack --preset <name>
    # Package the source code
    cpack --preset <name> --config CPackSourceConfig.cmake
    ```
To choose a specific generator you can append a semicolon seperated list via the `-G <generators>` flag
If no generator is specified then CPack will iterate through the list of generators and produce one package for each generator.
List of generators can be seen at the bottom of `cpack --help` or found at [cpack-generators(7)](https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html)

## All-in-one workflow
CMake provides a way to execute multiple steps in order.
   ```bash
    cmake --workflow --preset <name>
    ```


## Dependencies
- [Dear ImGui](https://github.com/ocornut/imgui/) ≥ 1.90.1
- [GLM](https://github.com/g-truc/glm) (used for now) ≥ 1.0.0
- [nlohmann::json](https://github.com/nlohmann/json) ≥ 3.11.3
- [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) ≥ 1.3.275
**OS Specific**
*Linux*
- [Wayland](https://gitlab.freedesktop.org/wayland/wayland) ≥ 1.22.0
- [wayland-protocols](https://gitlab.freedesktop.org/wayland/wayland-protocols) ≥ 1.33
and/or
- [X11 (XCB)](https://www.x.org/)

# For building
- [CMake](https://cmake.org/) ≥ 3.27
- [pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)
- Any C++ compiler that supports c++20/23

## Thanks to the following projects
Listed in no particular order

- [cpp_starter_project](https://github.com/lefticus/cpp_starter_project) and [adobe/lagrange](https://github.com/adobe/lagrange) for the knowledge and inspiring the CMake related stuff.
- [CMake](https://cmake.org/)
- [Ya-dola/mangadex-downloader-1](https://github.com/Ya-dola/mangadex-downloader-1) Mangadex downloader, inspired the beginning of this project

## License
This program is available to anybody free of charge, under the terms of `MIT` License (see [LICENSE](LICENSE)).
