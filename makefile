CC=ccache g++

all:
	g++ GameBoyEmu/*.cpp -I/usr/include/SDL2 --std=c++1y -O3 -march=native -o emu -lSDL2

cached:
	$(CC) GameBoyEmu/*.cpp -I/usr/include/SDL2 --std=c++1y -O3 -march=native -o emu -lSDL2
