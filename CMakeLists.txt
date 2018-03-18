cmake_minimum_required(VERSION 3.5)
project(emu)

set(CORE_SOURCES 
    GameBoyEmu/apu.cpp
    GameBoyEmu/cartrige.cpp
    GameBoyEmu/core.cpp
    GameBoyEmu/cpu.cpp
    GameBoyEmu/cpu_instructions.cpp
    GameBoyEmu/debugger.cpp
    GameBoyEmu/gpu.cpp
    GameBoyEmu/interrupts.cpp
    GameBoyEmu/joypad.cpp
    GameBoyEmu/mbc.cpp
    GameBoyEmu/mmu.cpp
    GameBoyEmu/noise_synth.cpp
    GameBoyEmu/ram.cpp
    GameBoyEmu/serial.cpp
    GameBoyEmu/square_synth.cpp
    GameBoyEmu/timer.cpp
    GameBoyEmu/wave_synth.cpp

    GameBoyEmu/main.cpp
)

set(CORE_HEADERS
    GameBoyEmu/apu.h
    GameBoyEmu/cartrige.h
    GameBoyEmu/core.h
    GameBoyEmu/cpu.h
    GameBoyEmu/debugger.h
    GameBoyEmu/gpu.h
    GameBoyEmu/IMemory.h
    GameBoyEmu/interrupts.h
    GameBoyEmu/joypad.h
    GameBoyEmu/mbc.h
    GameBoyEmu/mmu.h
    GameBoyEmu/noise_synth.h
    GameBoyEmu/ram.h
    GameBoyEmu/serial.h
    GameBoyEmu/square_synth.h
    GameBoyEmu/timer.h
    GameBoyEmu/wave_synth.h
)

set(UTIL_HEADERS
    GameBoyEmu/function.h
    GameBoyEmu/stdafx.h
    GameBoyEmu/definitions.h
    GameBoyEmu/bit_ops.h
)

set(TEST_SOURCES
    GameBoyEmu/sha256.cpp
)

set(TEST_HEADERS
    GameBoyEmu/sha256.h
)

set(PLATFORM_SOURCES
    GameBoyEmu/audio_postprocess.cpp
)

set(PLATFORM_HEADERS
    GameBoyEmu/audio_postprocess.h
)

option(USE_SDL "Use SDL2 library for platform stuff" ON)

#platform stuff (GCC)
set(CMAKE_CXX_STANDARD 14)

set(BUILD_FLAGS -Wall -Wextra -pedantic)
set(RELEASE_FLAGS -O3 -march=native -flto)
set(DEBUG_FLAGS -g)
set(SANITIZE_FLAGS -fno-omit-frame-pointer -fsanitize=address)

if(USE_SDL)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

#enable warnings for all targets
add_compile_options(${BUILD_FLAGS})

add_executable(release "")
target_sources(release PRIVATE ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(release PRIVATE ${RELEASE_FLAGS})
target_compile_definitions(release PRIVATE NDEBUG)
target_link_libraries(release ${SDL2_LIBRARIES})

add_executable(debug "")
target_sources(debug PRIVATE ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(debug PRIVATE ${DEBUG_FLAGS})
target_link_libraries(debug ${SDL2_LIBRARIES})

add_executable(sanitize "")
target_sources(sanitize PRIVATE ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(sanitize PRIVATE ${DEBUG_FLAGS} ${SANITIZE_FLAGS})
target_link_libraries(sanitize ${SDL2_LIBRARIES} ${SANITIZE_FLAGS}) #pass sanitize flags, to force ASAN linking

add_executable(auto_test "")
target_sources(auto_test PRIVATE ${TEST_SOURCES} ${TEST_HEADERS})
target_sources(auto_test PRIVATE ${CORE_SOURCES} ${CORE_HEADERS} ${UTIL_HEADERS} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
target_compile_options(auto_test PRIVATE ${DEBUG_FLAGS})
target_compile_definitions(auto_test PRIVATE ENABLE_AUTO_TESTS)
target_link_libraries(auto_test ${SDL2_LIBRARIES})
