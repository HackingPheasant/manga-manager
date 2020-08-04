# mangadex-dl
Mangadex downloader built with C++

## Build
1. Setup (Create build directory, run build generation system and grab dependencies)
    ```bash
    cmake . -B build
    ```
4. Build
    ```bash
    cmake --build build/
    ```
5. Install
    ```bash 
    cmake --install build # CMake 3.15+ only
    
    ```

Notes:
If you use `cmake --build` instead of directly calling the underlying build system you can use:
- `-v` for verbose builds (CMake 3.14+)
- `-j N` for parallel builds on N cores (CMake 3.12+)
- `--target` (Any CMake version) or `-t` (CMake 3.15+) to pick a target

Some common CMake options that may be useful:
- `-DCMAKE_BUILD_TYPE=` with the following options which may include `None`, `Debug`, `Release`, `RelWithDebInfo` or something else not listed
- `-DCMAKE_INSTALL_PREFIX ` to specify install location. Default is `/usr/local`
- `-DBUILD_SHARED_LIBS=` wither either `ON` or `OFF` to control the default for shared libraries

If you use make directly instead,  you can use:
- `VERBOSE=1 make` for verbose builds

## License
This program is available to anybody free of charge, under the terms of MIT License (see [LICENSE](LICENSE)).
