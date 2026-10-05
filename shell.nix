# SPDX-FileCopyrightText: © 2024 HackingPheasant <HackingPheasant@protonmail.com>
# SPDX-License-Identifier: MIT

# shell.nix - Configration to setup a build/dev enviroment. Most often will be
# used in conjunction with direnv (and nix_direnv). Alongside this file there
# should be an '.envrc' file that contain "use nix" for purpose. Then do 
# "direnv allow" and the enviroment will be set when changing into this directory.
#
# The pkgs.mkShell stdenv provides the following packages: 
# - GNU C Compiler (configured with C and C++ support),
# - GNU coreutils, findutils, diffutils, sed, grep, awk, tar and Make
# - gzip, bzip2 and x.
# - Bash. This is the shell used for all builders in the Nix Packages collection. 
#   Not using /bin/sh removes a large source of portability problems.
# - The patch command. And if on Linux, stdenv also includes the patchelf utility.
#
# NOTE: pkgs.mkShellNoCC is a variant that uses stdenvNoCC instead of stdenv as
# base environment. This is useful if no C compiler is needed in the shell 
# environment.
#
# Docs and Resources:
# https://nixos.org/manual/nixpkgs/stable/#sec-pkgs-mkShell
# https://nixos.org/manual/nixpkgs/stable/#sec-tools-of-stdenv
# https://nixos.org/manual/nixpkgs/stable/#ssec-stdenv-dependencies-overview
# https://github.com/NixOS/nixpkgs/issues/58624
# Note: Older (< 2018) scripts tend to use mkDerivation instead of mkShell.
# pkgs.mkShell is a specialized stdenv.mkDerivation that removes some repetition
# when using it with nix-shell (or nix develop).

{
  pkgs ? import <nixpkgs> { config = {}; overlays = []; },
}:

# Example on how to override 
# pkgs.mkShell.override { stdenv = clang19Stdenv; } {
pkgs.mkShell {
  strictDeps = true;

  # Executable packages to add to the nix-shell enviroment (aka nativeBuildInputs,
  # stuff only needed during the build process).
  packages = with pkgs; [
    ccache
    clang-tools
    cmakeCurses
    gdb
    lldb
    ninja
    pkg-config
    gdb
    valgrind
    (elfutils.override { enableDebuginfod = true; })
    vulkan-tools-lunarg
  ];

  # Packages to be linked against (dependencies needed at runtime)
  buildInputs = with pkgs; [
  ];

  # Add build dependencies of the listed derivations to the nix-shell enviroment
  # (Inherits inputs from other packages). Useful for if you want one super
  # shell.nix that brings in sub shell.nix's. Or for example if you have a
  # monorepo with multilple packages with thier own default.nix that you can
  # nix-shell and nix-build, you could then just have a top-level shell.nix
  # that would then pull all the dependencies together.
  inputsFrom = [
    (pkgs.callPackage ./package.nix {})
  ];

  # Bash statements that are executed by nix-shell
  #shellHook = ''
  #  export DEBUG=1
  #'';

  env = {
    NIX_SHELL_PRESERVE_PROMPT=1;
  };
}
