# manga-manager
Manga manger and downloader built with C++

## Build
1. Setup (Create build directory, run build generation system and grab dependencies)
    ```bash
    cmake . -B build
    ```
2. Build
    ```bash
    cmake --build build/
    ```
3. Install
    ```bash 
    cmake --install build # CMake 3.15+ only
    
    ```

**Notes:**
This code currently makes use of C++20 Ranges, and at the time of writing (2020-10-03) only GCC 10.1 and upwards has Ranges support.

If you use `cmake --build` instead of directly calling the underlying build system you can use:
- `-v` for verbose builds (CMake 3.14+)
- `--parallel N` or `-j N` for parallel builds on *N* amount of cores (CMake 3.12+)
- `--target` (Any CMake version) or `-t` (CMake 3.15+) to pick a target. E.g. `--target clean`

Some common CMake options that may be useful:
- `-DCMAKE_BUILD_TYPE=` with the following options which may include `None`, `Debug`, `Release`, `RelWithDebInfo` or something else not listed
    - `-- /p:configuration=Release` For Visual Studio
- `-DCMAKE_INSTALL_PREFIX ` to specify install location. Default is `/usr/local`
- `-DBUILD_SHARED_LIBS=` wither either `ON` or `OFF` to control the default for shared libraries

If you use make directly instead,  you can use:
- `VERBOSE=1 make` for verbose builds

To use a compiler different to the system default:
- In step one an example command would look like:
   `cmake . -B build -D CMAKE_C_COMPILER=gcc-10.2 -D CMAKE_CXX_COMPILER=g++-10.2`
- This is set in the first step since it effects everything else


## Thanks to the following projects
Listed in no particular order

- [cpp_starter_project](https://github.com/lefticus/cpp_starter_project) for some CMake knowledge
- [nlohmann/json](https://github.com/nlohmann/json/) C++ JSON library
- [whoshuu/cpr](https://github.com/whoshuu/cpr) C++ Requests wrapper around libcurl to simplify usage in code
- [fmtlib/fmt](https://github.com/fmtlib/fmt) A modern formatting library
- [CMake](https://cmake.org/)
- [bibo5088/mangadex-downloader](https://github.com/bibo5088/mangadex-downloader/) Mangadex downloader partially based off this project

## License
This program is available to anybody free of charge, under the terms of MIT License (see [LICENSE](LICENSE)).
