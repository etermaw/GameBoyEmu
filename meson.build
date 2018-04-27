project('GBE', 'cpp', version: '0.21')

add_project_arguments('-std=c++1y', '-Wall', '-Wextra', '-pedantic', language: 'cpp')

sdl = dependency('sdl2')
incdir = include_directories('GameBoyEmu')

core_src = ['GameBoyEmu/core/apu.cpp',
'GameBoyEmu/core/cartrige.cpp',
'GameBoyEmu/core/core.cpp',
'GameBoyEmu/core/cpu.cpp',
'GameBoyEmu/core/cpu_instructions.cpp',
'GameBoyEmu/core/debugger.cpp',
'GameBoyEmu/core/gpu.cpp',
'GameBoyEmu/core/interrupts.cpp',
'GameBoyEmu/core/joypad.cpp',
'GameBoyEmu/core/mbc.cpp',
'GameBoyEmu/core/mmu.cpp',
'GameBoyEmu/core/noise_synth.cpp',
'GameBoyEmu/core/ram.cpp',
'GameBoyEmu/core/serial.cpp',
'GameBoyEmu/core/square_synth.cpp',
'GameBoyEmu/core/timer.cpp',
'GameBoyEmu/core/wave_synth.cpp']

utils_src = ['GameBoyEmu/utils/sha256.cpp']

platform_src = ['GameBoyEmu/platform/sdl/audio_postprocess.cpp',
'GameBoyEmu/platform/sdl/gui.cpp',
'GameBoyEmu/platform/sdl/renderer.cpp']

prog = executable('release', sources: ['GameBoyEmu/main.cpp'] + core_src + utils_src + platform_src, dependencies: sdl, cpp_pch: 'GameBoyEmu/stdafx.h', include_directories : incdir)
