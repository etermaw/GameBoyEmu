CC = ccache g++

UNIV_OPTIONS = -std=c++1y -I/usr/include/SDL2
DEBUG_OPTIONS = -g -Wall -Wextra
RELEASE_OPTIONS = -O3 -march=native -flto
OPTIONS = $(UNIV_OPTIONS) $(RELEASE_OPTIONS)
LIBS = -lSDL2

R = $(CC) -c $(OPTIONS)

OBJECTS = apu.o cartrige.o cpu.o cpu_instructions.o gpu.o interrupts.o joypad.o main.o mbc.o mmu.o noise_synth.o ram.o square_synth.o timer.o wave_synth.o 


apu.o: GameBoyEmu/apu.cpp GameBoyEmu/apu.h GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/IMemory.h \
 GameBoyEmu/square_synth.h GameBoyEmu/wave_synth.h \
 GameBoyEmu/noise_synth.h
	$(R) GameBoyEmu/apu.cpp

cartrige.o: GameBoyEmu/cartrige.cpp GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/cartrige.h \
 GameBoyEmu/IMemory.h GameBoyEmu/mbc.h
	$(R) GameBoyEmu/cartrige.cpp

cpu.o: GameBoyEmu/cpu.cpp GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h \
 GameBoyEmu/cpu.h GameBoyEmu/mmu.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/cpu.cpp

cpu_instructions.o: GameBoyEmu/cpu_instructions.cpp GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/cpu.h \
 GameBoyEmu/mmu.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/cpu_instructions.cpp

gpu.o: GameBoyEmu/gpu.cpp GameBoyEmu/gpu.h GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/IMemory.h \
 GameBoyEmu/interrupts.h
	$(R) GameBoyEmu/gpu.cpp

interrupts.o: GameBoyEmu/interrupts.cpp GameBoyEmu/interrupts.h \
 GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/interrupts.cpp

joypad.o: GameBoyEmu/joypad.cpp GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/joypad.h \
 GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/joypad.cpp

main.o: GameBoyEmu/main.cpp GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h \
 GameBoyEmu/cpu.h GameBoyEmu/mmu.h GameBoyEmu/IMemory.h \
 GameBoyEmu/cpu_instructions.h GameBoyEmu/ram.h GameBoyEmu/cartrige.h \
 GameBoyEmu/mbc.h GameBoyEmu/interrupts.h GameBoyEmu/gpu.h \
 GameBoyEmu/timer.h GameBoyEmu/joypad.h GameBoyEmu/apu.h \
 GameBoyEmu/square_synth.h GameBoyEmu/wave_synth.h \
 GameBoyEmu/noise_synth.h GameBoyEmu/debugger.h
	$(R) GameBoyEmu/main.cpp

mbc.o: GameBoyEmu/mbc.cpp GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h \
 GameBoyEmu/mbc.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/mbc.cpp

mmu.o: GameBoyEmu/mmu.cpp GameBoyEmu/mmu.h GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/mmu.cpp

noise_synth.o: GameBoyEmu/noise_synth.cpp GameBoyEmu/noise_synth.h \
 GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h
	$(R) GameBoyEmu/noise_synth.cpp

ram.o: GameBoyEmu/ram.cpp GameBoyEmu/ram.h GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h GameBoyEmu/IMemory.h
	$(R) GameBoyEmu/ram.cpp

square_synth.o: GameBoyEmu/square_synth.cpp GameBoyEmu/square_synth.h \
 GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h
	$(R) GameBoyEmu/square_synth.cpp

timer.o: GameBoyEmu/timer.cpp GameBoyEmu/stdafx.h \
 GameBoyEmu/definitions.h GameBoyEmu/bit_ops.h GameBoyEmu/function.h \
 GameBoyEmu/timer.h GameBoyEmu/IMemory.h GameBoyEmu/interrupts.h
	$(R) GameBoyEmu/timer.cpp

wave_synth.o: GameBoyEmu/wave_synth.cpp GameBoyEmu/wave_synth.h \
 GameBoyEmu/stdafx.h GameBoyEmu/definitions.h \
 GameBoyEmu/bit_ops.h GameBoyEmu/function.h
	$(R) GameBoyEmu/wave_synth.cpp

debug: $(OBJECTS)

release: $(OBJECTS)
	 $(CC) $(OPTIONS) $(OBJECTS) -o emu $(LIBS)

clean:
	rm *.o
