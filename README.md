# GameBoyEmu
Game Boy Color multiplatform emulator written in C++ with performance in mind. Include features like:
* DMG and CGB emulation
* battery save support
* accurate audio emulation
* accurate GPU emulation
* built-in debugger
* frame blending

Currently emulator passes most of [blaargh tests](https://github.com/retrio/gb-test-roms) and 65 of 82 [gekkio`s tests](https://github.com/Gekkio/mooneye-gb/tree/master/tests)

Emulator supports 2 versions: lightweight SDL version (without window), and heavy Qt 5 + OpenGL version (window with more options).

## Building
Project require [Meson](https://github.com/mesonbuild/meson) build system, [Ninja](https://github.com/ninja-build/ninja), Python 3
as basic requirement. You can get them using commands:
`sudo apt install ninja-build`
`pip3 install --user meson`

For sdl version you will also need SDL2 (`sudo apt install libsdl2-dev`), for qt version you will need qt 5.2 or higher
(`sudo apt install qt5-default`).

### Linux
1. Clone project
2. `cd <project dir>`
3. Create new build dir using Meson:
`meson <build dir name>`
4. `cd <build dir name>`
5. configure project:
-sdl version `meson configure -Dbuildtype=release -Db_lto=true -Dplatform_handler=sdl2`
-qt version `meson configure -Dbuildtype=release -Db_lto=true -Dplatform_handler=qt5`

6. build project
type `ninja`

### Windows
steps 1-5 are the same as on linux.

6. create MS Build file ([link](http://mesonbuild.com/Using-with-Visual-Studio.html) to manual)
7. compile

Emulator was compiled and tested on Ubuntu, Windows 7 (32 bit) and Windows 10 (64 bit).
