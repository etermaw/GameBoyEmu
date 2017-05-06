#include "stdafx.h"
#include "cpu.h"
#include "cpu_instructions.h"
#include "mmu.h"
#include "ram.h"
#include "cartrige.h"
#include "interrupts.h"
#include "gpu.h"
#include "timer.h"
#include "joypad.h"
#include "apu.h"
#include "debugger.h"

struct TestReader final : public IMemory
{
	u8 a;

	public:
		TestReader() : a() {}

		u8 read_byte(u16 adress, u32 unused) override
		{
			return 0;
			//return 0xFF;
		}

		void write_byte(u16 adress, u8 val, u32 unused) override
		{
			if (adress == 0xFF01)
				a = val;

			if (adress == 0xFF02 && val == 0x81)
				printf("%c", a);
		}
};

struct SpeedSwitch : public IMemory
{
	bool double_speed = false;
	bool switch_speed = false;

	u8 read_byte(u16 adress, u32 unused) override
	{
		if (adress == 0xFF4D)
		{
			auto ret = change_bit(0xFF, double_speed, 7);
			return change_bit(ret, switch_speed, 0);
		}

		else
			return 0xFF;
	}

	void write_byte(u16 adress, u8 val, u32 key) override
	{
		if (adress == 0xFF4D)
		{
			if (key == 0xFFFFFFFF && val == 0xFF)
			{
				switch_speed = false;
				double_speed = !double_speed;
			}

			else
				switch_speed = check_bit(val, 0);
		}

		//else ignore
	}
};

int main(int argc, char *argv[])
{
	Interrupts ints;
	Timer timer(ints);
	MMU mmu;
	Ram ram;
	Cartrige cart;
	TestReader tr; //testing
	Joypad joypad;
	CPU cpu(mmu);
	Gpu gpu(ints);
	APU apu;
	SpeedSwitch speed;

	Debugger debugger;
	debugger.attach_mmu(make_function(&MMU::read_byte, &mmu), make_function(&MMU::write_byte, &mmu));

	cpu.attach_debugger(debugger.get_cpu());
	mmu.attach_debug_callback(make_function(&Debugger::check_memory_access, &debugger));

	SDL_Window* window = SDL_CreateWindow("Test",
											SDL_WINDOWPOS_UNDEFINED,
											SDL_WINDOWPOS_UNDEFINED,
											160 * 3,
											144 * 3,
											0);

	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_Event ev;

	std::string file_name;

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		if (cart.load_cartrige(file_name))
		{
			std::cout << "Cartrige loaded!\n";
			break;
		}

		std::cout << "Failed to load cartrige!\n";
	}

	mmu.register_chunk(0, 0x7FFF, cart.get_memory_interface());
	mmu.register_chunk(0x8000, 0x9FFF, &gpu); //vram
	mmu.register_chunk(0xA000, 0xBFFF, cart.get_memory_interface());
	mmu.register_chunk(0xC000, 0xFDFF, &ram);
	mmu.register_chunk(0xFE00, 0xFE9F, &gpu); //oam tables
	mmu.register_chunk(0xFF00, 0xFF00, &joypad);//input keys register
	mmu.register_chunk(0xFF01, 0xFF02, &tr); //TEST READER!!!!
	mmu.register_chunk(0xFF04, 0xFF07, &timer);//timer controls
	mmu.register_chunk(0xFF0F, 0xFF0F, &ints);//interrupts flags

	mmu.register_chunk(0xFF10, 0xFF3F, &apu); //APU registers + wave RAM 

	mmu.register_chunk(0xFF40, 0xFF4B, &gpu); //gpu control regs
	mmu.register_chunk(0xFF4D, 0xFF4D, &speed); //speed switch (CGB)
	mmu.register_chunk(0xFF80, 0xFFFE, &ram); //high ram
	mmu.register_chunk(0xFFFF, 0xFFFF, &ints); //interrupts

	gpu.set_ram_dma(ram.get());

	bool spin = true;
	const u32 key_map[8] = { SDLK_RIGHT, SDLK_LEFT, SDLK_UP, SDLK_DOWN, SDLK_a, SDLK_b, SDLK_KP_ENTER, SDLK_s };

	while (spin)
	{
		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
			{
				u32 key_code = 0;

				while (key_map[key_code] != ev.key.keysym.sym && key_code < 8)
					++key_code;

				if (key_code < 8)
				{
					if (ev.type == SDL_KEYDOWN)
						joypad.push_key(static_cast<KEYS>(key_code));

					else
						joypad.release_key(static_cast<KEYS>(key_code));
				}
			}
			
			else if (ev.type == SDL_QUIT)
				spin = false;
		}

		//auto start = std::chrono::high_resolution_clock::now();
		while (!gpu.is_entering_vblank()) //if someone turn off lcd, this loop may spin forever
		{
			u32 sync_cycles = 0;

			if (ints.is_any_raised())
			{
				cpu.unhalt();

				if (cpu.is_interrupt_enabled())
					sync_cycles = cpu.handle_interrupt(ints.get_first_raised());
			}

			debugger.step();

			sync_cycles += cpu.step();
			sync_cycles += gpu.step(sync_cycles >> speed.double_speed);
			apu.step(sync_cycles >> speed.double_speed);
			timer.step(sync_cycles);
		}

		auto ptr = gpu.get_frame_buffer();
		void* pixels = nullptr;
		int pitch = 0;

		SDL_LockTexture(tex, NULL, &pixels, &pitch);
		std::memcpy(pixels, ptr, sizeof(u32) * 160 * 144);
		SDL_UnlockTexture(tex);

		SDL_RenderClear(rend);
		SDL_RenderCopy(rend, tex, NULL, NULL);
		SDL_RenderPresent(rend);

		//auto end = std::chrono::high_resolution_clock::now();
		//auto dur = (end - start).count();

		/*if ((dur / 1000000) < 16)
			SDL_Delay(16 - (dur / 1000000));*/
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);

	return 0;
}