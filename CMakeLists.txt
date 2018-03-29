cmake_minimum_required(VERSION 3.5)
project(emu)

file(GLOB CORE_SOURCES GameBoyEmu/core/*.cpp)
file(GLOB CORE_HEADERS GameBoyEmu/core/*.h)

file(GLOB UTIL_SOURCES GameBoyEmu/utils/*.cpp)
file(GLOB UTIL_HEADERS GameBoyEmu/utils/*.h)

set(TEST_SOURCES
    GameBoyEmu/sha256.cpp
    GameBoyEmu/platform/linux/test.cpp
)

set(TEST_HEADERS
    GameBoyEmu/sha256.h
    GameBoyEmu/platform/linux/test.h
)


option(USE_SDL "Use SDL2 library for platform stuff" ON)

if(USE_SDL)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    file(GLOB PLATFORM_SOURCES GameBoyEmu/platform/sdl/*.cpp)
    file(GLOB PLATFORM_HEADERS GameBoyEmu/platform/sdl/*.h)
    link_libraries(${SDL2_LIBRARIES})
endif()

set(PLATFORM_HEADERS ${PLATFORM_HEADERS} GameBoyEmu/platform/dispatch.h)

#platform stuff (GCC)
set(CMAKE_CXX_STANDARD 14)

set(BUILD_FLAGS -Wall -Wextra -pedantic)
set(RELEASE_FLAGS -O3 -march=native -flto)
set(DEBUG_FLAGS -g)
set(SANITIZE_FLAGS -fno-omit-frame-pointer -fsanitize=address)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

#enable warnings for all targets
add_compile_options(${BUILD_FLAGS})
include_directories(GameBoyEmu)

add_executable(release "")
target_sources(release PRIVATE GameBoyEmu/main.cpp ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(release PRIVATE ${RELEASE_FLAGS})
target_compile_definitions(release PRIVATE NDEBUG)

add_executable(debug "")
target_sources(debug PRIVATE GameBoyEmu/main.cpp ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(debug PRIVATE ${DEBUG_FLAGS})

add_executable(sanitize "")
target_sources(sanitize PRIVATE GameBoyEmu/main.cpp ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(sanitize PRIVATE ${DEBUG_FLAGS} ${SANITIZE_FLAGS})
target_link_libraries(sanitize ${SANITIZE_FLAGS}) #pass sanitize flags, to force ASAN linking

add_executable(auto_test "")
target_sources(auto_test PRIVATE ${TEST_SOURCES} ${TEST_HEADERS})
target_sources(auto_test PRIVATE GameBoyEmu/main.cpp ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(auto_test PRIVATE ${DEBUG_FLAGS})
target_compile_definitions(auto_test PRIVATE ENABLE_AUTO_TESTS)
