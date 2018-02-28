SRC_DIR=GameBoyEmu
BUILD_DIR=build
EXE=emu

SRC=$(wildcard $(SRC_DIR)/*.cpp)
OBJ=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEP=$(OBJ:%o=%d)

CXX= ccache g++
CXXFLAGS= -std=c++1y -I/usr/include/SDL2
LIBS= -lSDL2

all: release

release: CXXFLAGS += -O3 -march=native -flto
release: dir executable

debug: CXXFLAGS += -Wall -Wextra -pedantic -g
debug: dir executable

debug_san: CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
debug_san: dir debug

test: CXXFLAGS += -DENABLE_AUTO_TESTS
test: dir debug

dir:
	mkdir -p $(BUILD_DIR)

executable: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(EXE) $^ $(LIBS)

-include $(DEP)

$(OBJ): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -MMD -c $(CXXFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d

