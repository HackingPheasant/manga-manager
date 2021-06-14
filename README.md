# manga-manager
Manga manger and downloader built with C++

## Build
1. Configure
    ```bash
    cmake --preset <name>
    ```
2. Build
    ```bash
    cmake --build --preset <name>
    ```
3. Install
    ```bash
    cmake --install --preset <name> # CMake 3.15+ only
    ```

4. Test (Optional)
    ```bash
    ctest --preset <name>
    ```

To list the available presets that can be used, run `--list-presets`.
Some available options include `dev`, `debug`, `release`, `relwithdebinfo` etc.

**Notes:**
If you use `cmake --build` instead of directly calling the underlying build system you can use:
- `--parallel N` or `-j N` for parallel builds on *N* amount of cores (CMake 3.12+)
- `--target` (Any CMake version) or `-t` (CMake 3.15+) to pick a target. E.g. `--target clean`
- `-v` for verbose builds (CMake 3.14+)

If you use make directly instead,  you can use:
- `VERBOSE=1 make` for verbose builds

To use a compiler different to the system default:
- In the configure step, append the following:
   `-D CMAKE_C_COMPILER=gcc-10.2 -D CMAKE_CXX_COMPILER=g++-10.2`

## Thanks to the following projects
Listed in no particular order

- [cpp_starter_project](https://github.com/lefticus/cpp_starter_project) and [adobe/lagrange](https://github.com/adobe/lagrange) for the knowledge and inspiring the CMake related stuff.
- [nlohmann/json](https://github.com/nlohmann/json/) C++ JSON library
- [whoshuu/cpr](https://github.com/whoshuu/cpr) C++ Requests wrapper around [curl](https://github.com/curl/curl) to simplify usage in code
- [fmtlib/fmt](https://github.com/fmtlib/fmt) A modern formatting library
- [CMake](https://cmake.org/)
- [bibo5088/mangadex-downloader](https://github.com/bibo5088/mangadex-downloader/) Mangadex downloader, inspired the beginning of this project

## License
This program is available to anybody free of charge, under the terms of `MIT` License (see [LICENSE](LICENSE)).
