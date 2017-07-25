SRC=$(wildcard GameBoyEmu/*.cpp)
OBJ=$(subst .cpp,.o,$(SRC))
CXX= ccache g++
CXXFLAGS= -std=c++1y -I/usr/include/SDL2
LIBS= -lSDL2

all: release 

release: CXXFLAGS += -O3 -march=native -flto
release: executable

debug: CXXFLAGS += -Wall -Wextra -pedantic -g
debug: executable

executable: $(OBJ)
	$(CXX) $(CXXFLAGS) -o emu $^ $(LIBS)

$(OBJ): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm $(OBJ)
