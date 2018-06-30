project('GBE', 'cpp', version: '0.21', default_options: ['cpp_std=c++14', 'warning_level=3'])

deps = []
incdir = [include_directories('GameBoyEmu')]

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

test_src = ['GameBoyEmu/platform/linux/test.cpp', 'GameBoyEmu/platform/linux/main.cpp']

args = []
final_src = []

auto_tester_module = get_option('auto_test_module')

if auto_tester_module == 'basic'
    final_src += test_src + utils_src

elif auto_tester_module == 'setup'
    deps += dependency('sdl2')

    final_src += test_src + utils_src + ['GameBoyEmu/platform/sdl/gui.cpp', 'GameBoyEmu/platform/sdl/renderer.cpp']
    args += ['-DENABLE_TEST_DISPLAY']
    
else
    if get_option('platform_handler') == 'sdl2'
        deps += dependency('sdl2')

        platform_src = ['GameBoyEmu/platform/sdl/audio_postprocess.cpp',
                        'GameBoyEmu/platform/sdl/gui.cpp',
                        'GameBoyEmu/platform/sdl/renderer.cpp',
                        'GameBoyEmu/platform/sdl/main.cpp']


        final_src += core_src + platform_src

    else
        qt5 = import('qt5')
        deps += dependency('qt5', modules: ['Core', 'Gui', 'Widgets'])

        moc = qt5.preprocess(moc_headers: ['GameBoyEmu/platform/qt/mainwindow.h', 'GameBoyEmu/platform/qt/keymap.h', 'GameBoyEmu/platform/qt/keycaptureedit.h', 'GameBoyEmu/platform/qt/corethread.h', 'GameBoyEmu/platform/qt/renderwidget.h'],
                            ui_files: ['GameBoyEmu/platform/qt/mainwindow.ui', 'GameBoyEmu/platform/qt/keymap.ui'],
                            qresources: ['GameBoyEmu/platform/qt/resources.qrc'])

        platform_src = ['GameBoyEmu/platform/qt/main.cpp',
                        'GameBoyEmu/platform/qt/mainwindow.cpp',
                        'GameBoyEmu/platform/qt/keymap.cpp',
                        'GameBoyEmu/platform/qt/keycaptureedit.cpp',
                        'GameBoyEmu/platform/qt/renderwidget.cpp',
                        'GameBoyEmu/platform/qt/corethread.cpp']

        incdir += include_directories('GameBoyEmu/platform/qt') #it`s required for compiling custom KeyEdit

        final_src += moc + core_src + platform_src
    endif
endif

prog = executable('emu', sources: final_src, dependencies: deps, cpp_args: args, cpp_pch : ['GameBoyEmu/pch/stdafx.h', 'GameBoyEmu/pch/stdafx.cpp'], include_directories : incdir)
