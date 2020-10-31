# manga-manager
Manga manger and downloader built with C++

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

**Notes:**
By default (if you don't set environment variables `CC` and `CXX`), the system default compiler will be used.
This code currently makes use of C++20 Ranges, and at the time of writing (2020-10-03) only GCC 10.1 and upwards has Ranges support.

<details>
<summary>Commands for setting the compilers </summary>
Substitute your desired compiler (`clang`, `gcc`, etc) as necessary when following.

- ***NIX Based:**
    - **Temporarily** *(only for the current shell)***:**
    
        Run one of the following in the terminal:
        - clang
        ```bash
        CC=clang
        CXX=clang++
        ```
        - gcc
        ```bash
        CC=gcc
        CXX=g++
        ```
    - **Permanent:**
    
        Set in your shell config/startup file, the following example shows how to setup clang as the default for the bash shell

        Open `~/.bashrc` using your text editor:
        ```bash
        gedit ~/.bashrc
        ```
        Add `CC` and `CXX` to point to the compilers:
        ```basH
        export CC=clang
        export CXX=clang++
        ```
        Save and close the file.

- **Windows:**
    - **Permanent:**
    
        Run one of the following in PowerShell:
        - Visual Studio generator and compiler (cl)
        ```powershell
        [Environment]::SetEnvironmentVariable("CC", "cl.exe", "User")
        [Environment]::SetEnvironmentVariable("CXX", "cl.exe", "User")
        refreshenv
        ```
        Set the architecture using [vsvarsall](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019#vcvarsall-syntax):
        ```powershell
        vsvarsall.bat x64
        ```
        - clang
        ```powershell
        [Environment]::SetEnvironmentVariable("CC", "clang.exe", "User")
        [Environment]::SetEnvironmentVariable("CXX", "clang++.exe", "User")
        refreshenv
        ```
        - gcc
        ```powershell
        [Environment]::SetEnvironmentVariable("CC", "gcc.exe", "User")
        [Environment]::SetEnvironmentVariable("CXX", "g++.exe", "User")
        refreshenv
        ```
    - **Temporarily** *(only for the current shell)***:**
    
        Run one of the following in PowerShell:
        - clang
        ```powershell
        $Env:CC="clang.exe"
        $Env:CXX="clang++.exe"
        ```
        - gcc
        ```powershell
        $Env:CC="gcc.exe"
        $Env:CXX="g++.exe"
        ```

</details>

If you use `cmake --build` instead of directly calling the underlying build system you can use:
- `-v` for verbose builds (CMake 3.14+)
- `--parallel N` or `-j N` for parallel builds on N cores (CMake 3.12+)
- `--target` (Any CMake version) or `-t` (CMake 3.15+) to pick a target. E.g. `--target clean`

Some common CMake options that may be useful:
- `-DCMAKE_BUILD_TYPE=` with the following options which may include `None`, `Debug`, `Release`, `RelWithDebInfo` or something else not listed
    - `-- /p:configuration=Release` For Visual Studio
- `-DCMAKE_INSTALL_PREFIX ` to specify install location. Default is `/usr/local`
- `-DBUILD_SHARED_LIBS=` wither either `ON` or `OFF` to control the default for shared libraries

If you use make directly instead,  you can use:
- `VERBOSE=1 make` for verbose builds

## Thanks for following projects
Listed in no particular order

- [cpp_starter_project](https://github.com/lefticus/cpp_starter_project) for some CMake knowledge
- [nlohmann/json](https://github.com/nlohmann/json/) C++ JSON library
- [whoshuu/cpr](https://github.com/whoshuu/cpr) C++ Requests wrapper around libcurl to simplify usage in code
- [CMake](https://cmake.org/)
- [bibo5088/mangadex-downloader](https://github.com/bibo5088/mangadex-downloader/) Mangadex downloader partially based off this project

## License
This program is available to anybody free of charge, under the terms of MIT License (see [LICENSE](LICENSE)).
