# Architecture

This document describes the high-level architecture of manga-manager.

## Quick Overview
This project is intended to be a testing and learning ground for various technologies, APIs and ideas with a somewhat aimed goal of being a 2D/3D File manager aimed specially for Manga and related.

Currently only messing with the low level building blocks and concepts and no working program currently exists (as of August 2024).

**NOTES:** This document is still in a TODO state, and will be till we directly link each section to its own documents pages, for quicker access to deeper reading.

## Code Map
This aims to give a brief overview of various important directories.
Please refer to `[docs/](docs/)` for a more detailed information on things.

### `cmake/` and `cmake/3rd-party/`

`cmake/` is where the build system functions, sanitizers and options are stored or configured.
`cmake/3rd-party/` is where we acquire our dependencies, either via CMake FetchContent, FindLibrary or PkgConfig

### `src/core`
Contains functions or wrappers around stuff that is used in the program throughout.
Below is a quick (but not exhaustive) overview of whats available:
TODO:
Curl wrapper (potentially async via coroutines) found in http.cpp
Vulkan Instantiation.

### `src/os`
Contains the high level interfaces that the program may use to interact with OS level components. Such as Window creation, event loop (TODO), input handling (TODO), audio handling (TODO) and secret handling (TODO).

Each subfolder found inside is named after the OS for which you can find the implementation details inside of.
`src/os/linux/{x11/wayland}` is a tad special as we are instead partially targeting specific Display protocols that are common in the linux ecosystem. Such as Wayland and X11. These particular folders may potentially move to a more generic \*nix/\*BSD type folder if so required.

### `src/providers/`
In here you'll find code to handle different 3rd party API's, there is currently no unified interface yet for interacting with these API's yet so you'll have to independently check what available to use.

### `src/ui/`
Contains the Render (In-progress) and UI (TODO) code and assets.

The Render builds atop of the building blocks Vulkan code found in `src/core/` (TODO) with its aim to be able to handle normal 2D UI that can change into a 3D look when requested, how this would actually look and work in real life has yet to be decided on, but it is one of the goals I would like to try and achieve. It would be a different way to browse and access files and data.
