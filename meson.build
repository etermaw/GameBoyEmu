project('GBE', 'cpp', version: '0.21', default_options: ['cpp_std=c++14', 'warning_level=3'])

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

test_src = ['GameBoyEmu/platform/linux/test.cpp']

args = []
final_src = []

if get_option('auto_test') == true
    final_src += test_src + utils_src
    args = ['-DENABLE_AUTO_TESTS']
endif

if get_option('platform_handler') == 'sdl2'
    platform_src = ['GameBoyEmu/platform/sdl/audio_postprocess.cpp',
                    'GameBoyEmu/platform/sdl/gui.cpp',
                    'GameBoyEmu/platform/sdl/renderer.cpp',
                    'GameBoyEmu/platform/sdl/main.cpp']


    final_src += core_src + platform_src
    prog = executable('emu', sources: final_src, dependencies: sdl, cpp_args: args, cpp_pch : ['GameBoyEmu/pch/stdafx.h', 'GameBoyEmu/pch/stdafx.cpp'], include_directories : incdir)

else
    qt5 = import('qt5')
    qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])

    moc = qt5.preprocess(moc_headers: ['GameBoyEmu/platform/qt/mainwindow.h', 'GameBoyEmu/platform/qt/keymap.h'],
                         ui_files: ['GameBoyEmu/platform/qt/mainwindow.ui', 'GameBoyEmu/platform/qt/keymap.ui'])

    platform_src = ['GameBoyEmu/platform/qt/main.cpp', 'GameBoyEmu/platform/qt/mainwindow.cpp','GameBoyEmu/platform/qt/keymap.cpp']

    final_src += moc
    final_src += core_src + platform_src
    prog = executable('emu', sources: final_src, dependencies: qt5_dep, cpp_args: args, cpp_pch : ['GameBoyEmu/pch/stdafx.h', 'GameBoyEmu/pch/stdafx.cpp'], include_directories : incdir)

endif
