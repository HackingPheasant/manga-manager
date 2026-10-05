# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Bare bones project skeleton, including related CMake modules and code
- Basic renderer implemented in Vulkan
- Basic client implementation for Wayland and X11 Display protocols
- Introduced an OS antagonistic abstraction for creating a window/client
- Basic README and docs/ folder
- Noted down what LICENSES are used by dependencies of this project in the top level CMakeLists.txt file and related LICENSES/ directory
- Introduced .clang-format and .clang-tidy configurations
- Introduced package.nix and shell.nix to ease development and building on NixOS

[unreleased]: https://github.com/HackingPheasant/manga-manager/compare/c461c8b7a8685e43e739d1335d9a6e7c242c4246...HEAD
