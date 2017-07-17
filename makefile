SRC=$(wildcard GameBoyEmu/*.cpp)
OBJ=$(subst .cpp,.o,$(SRC))
CXX=ccache g++
CXXFLAGS=-std=c++1y -O3 -march=native -flto -I/usr/include/SDL2
LIBS=-lSDL2

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o emu $^ $(LIBS)

$(OBJ): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
