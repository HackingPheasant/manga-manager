# SPDX-FileCopyrightText: © 2024 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

{
  lib,
  stdenv,
  fetchFromGitHub,
  # nativeBuildInputs
  cmake,
  glslang,
  ninja,
  pkg-config,
  wayland-scanner,
  # buildInputs
  glm,
  libffi,
  nlohmann_json,
  wayland,
  wayland-protocols,
  vulkan-headers,
  vulkan-loader,
  xorg,

  withWayland ? true,
  withX11 ? true,
}:

stdenv.mkDerivation rec {
  pname = "manga-manager";
  version = "0.0.1";

  src = builtins.path { path = ./.; name = "manga-manager"; };

  #src = fetchFromGitHub {
  #  owner = "HackingPheasant";
  #  repo = pname;
  #  rev = version;
  #  sha256 = pkg.lib.fakeHash;"
  #};

  strictDeps = true;

  # Excutable packages used to produce packages also used at build time,
  # if the dependency doesn't care about the target platform put it in
  # nativeBuildInputs instead.
  depsBuildBuild = [
    pkg-config
  ];

  # Executable packages, stuff only needed during the build process
  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
  ]
  ++ lib.optionals withWayland [
    wayland-scanner 
  ];

  # Packages to be linked against (dependencies needed at runtime)
  buildInputs = [
    glm
    glslang
    libffi # Wayland *.pc files complain about needing this
    nlohmann_json
    vulkan-headers
    vulkan-loader
  ]
  ++ lib.optionals withWayland [
    libffi # Wayland *.pc files complain about needing this
    wayland
    #wayland-scanner # Here instead of nativeBuildInputs so the pkg-config .pc files are hooked properly by nix
    wayland-protocols
  ]
  ++ lib.optionals withX11 [
    #xorg.libX11 # Am trying to drop this as I don't make use of xlib, will drop once I write my own copy of FindXCB (based of FindX11)
    xorg.libxcb
    #libxcb
    xorg.xcbutil
    #libxcb-util
    #xorg.xorgproto # Potentially needed for xwayland stuff
  ];

  meta = {
    description = "Manga manger and downloader built with C++";
    longDescription = ''
      This started out as a manage downloader and exploded out into more, it is
      a way for me learn C++ now.
    '';
    homepage = "https://github.com/HackingPheasant/manga-manager";
    downloadPage = "https://github.com/HackingPheasant/manga-manager/releases/";
    changelog = "https://github.com/HackingPheasant/manga-manager/CHANGELOG";
    license = lib.licenses.mit;
    maintainers = with lib.maintainers; [ HackingPheasant ];
    platforms = lib.platforms.all;
  };
}
