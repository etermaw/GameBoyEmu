all:
	g++ GameBoyEmu/*.cpp -I/usr/include/SDL2 --std=c++1y -O3 -march=native -o emu -lSDL2
