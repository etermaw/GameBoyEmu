project('GBE', 'cpp', version: '0.21')

sdl = dependency('sdl2')
incdir = include_directories('GameBoyEmu')

core_src = ['GameBoyEmu/core/apu.cpp',
'GameBoyEmu/core/apu.h',
'GameBoyEmu/core/cartrige.cpp',
'GameBoyEmu/core/cartrige.h',
'GameBoyEmu/core/core.cpp',
'GameBoyEmu/core/core.h',
'GameBoyEmu/core/cpu.cpp',
'GameBoyEmu/core/cpu.h',
'GameBoyEmu/core/cpu_instructions.cpp',
'GameBoyEmu/core/debugger.cpp',
'GameBoyEmu/core/debugger.h',
'GameBoyEmu/core/gpu.cpp',
'GameBoyEmu/core/gpu.h',
'GameBoyEmu/core/IMemory.h',
'GameBoyEmu/core/interrupts.cpp',
'GameBoyEmu/core/interrupts.h',
'GameBoyEmu/core/joypad.cpp',
'GameBoyEmu/core/joypad.h',
'GameBoyEmu/core/mbc.cpp',
'GameBoyEmu/core/mbc.h',
'GameBoyEmu/core/mmu.cpp',
'GameBoyEmu/core/mmu.h',
'GameBoyEmu/core/noise_synth.cpp',
'GameBoyEmu/core/noise_synth.h',
'GameBoyEmu/core/ram.cpp',
'GameBoyEmu/core/ram.h',
'GameBoyEmu/core/serial.cpp',
'GameBoyEmu/core/serial.h',
'GameBoyEmu/core/square_synth.cpp',
'GameBoyEmu/core/square_synth.h',
'GameBoyEmu/core/timer.cpp',
'GameBoyEmu/core/timer.h',
'GameBoyEmu/core/wave_synth.cpp',
'GameBoyEmu/core/wave_synth.h']

utils_src = ['GameBoyEmu/utils/bit_ops.h',
'GameBoyEmu/utils/definitions.h',
'GameBoyEmu/utils/function.h',
'GameBoyEmu/utils/logger.h',
'GameBoyEmu/utils/sha256.cpp',
'GameBoyEmu/utils/sha256.h']

platform_src = ['GameBoyEmu/platform/sdl/audio_postprocess.cpp',
'GameBoyEmu/platform/sdl/audio_postprocess.h',
'GameBoyEmu/platform/sdl/gui.cpp',
'GameBoyEmu/platform/sdl/gui.h',
'GameBoyEmu/platform/sdl/renderer.cpp',
'GameBoyEmu/platform/sdl/renderer.h']

prog = executable('release', sources: ['GameBoyEmu/main.cpp'] + core_src + utils_src + platform_src, dependencies: sdl, cpp_args: '-std=c++1y', cpp_pch: 'GameBoyEmu/stdafx.h', include_directories : incdir)
