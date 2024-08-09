# SDF ENGINE
> A cute renderer for SDF objects

## Build

### Install dependencies

> [SFML >= 2.5.1](https://github.com/SFML/SFML)

> OpenGL

> [Dear ImGui](https://github.com/ocornut/imgui)

> [ImGui SFML](https://github.com/SFML/imgui-sfml)

SDF Engine is only dependent on SFML/OpenGL, so make sure those are accessible to your build environment. You will also need a command line C++ compiler, as well as make and preferably GDB. There is already a nix flake provided to do this if you run NixOS.

Next, you will need to create the imgui directory in the main folder. Simply copy/paste the source files from the [imgui github](https://github.com/ocornut/imgui), as well as the source files from the SFML backend, [imgui sfml backend](https://github.com/SFML/imgui-sfml). Copy/paste both of these into a folder entitled just imgui.

### Set compiler

Set the compiler to your preferred c++ compiler by simply altering the first line of the Makefile. CFLAGS contains the flags used for a build.

### Make commands

To compile the program, you will first need to compile imgui. To do this, simply write `make cimgui`.

After this, simply calling `make` will compile the program. `make run` will compile and run the program. `make clean` will clean the directory and allow you to perform a fresh recompile. `make debug` will make a debug version for gdb. Lastly, `make release` will generate a release version of the program, which just gets rid of the unnecessary terminal window.
