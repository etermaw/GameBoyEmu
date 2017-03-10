#include "stdafx.h"
#include "cpu.h"
#include "mmu.h"
#include "ram.h"
#include "cartrige.h"
#include "interrupts.h"
#include "gpu.h"
#include "timer.h"
#include "joypad.h"

struct TestReader final : public IMemory
{
	u8 a;

	public:
		TestReader() : a() {}

		u8 read_byte(u16 adress) override
		{
			return 0xFF;
		}

		void write_byte(u16 adress, u8 val) override
		{
			if (adress == 0xFF01)
				a = val;

			if (adress == 0xFF02 && val == 0x81)
				printf("%c", a);
		}
};

int main(int argc, char *argv[])//int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	Timer timer;
	Interrupts ints;
	MMU mmu;
	CPU_Debugger<CPU> cpu;//CPU cpu;
	Ram ram;
	Cartrige cart;
	TestReader tr; //testing
	Joypad joypad;
	Gpu gpu;

	SDL_Window* window = SDL_CreateWindow("Test",
											SDL_WINDOWPOS_UNDEFINED,
											SDL_WINDOWPOS_UNDEFINED,
											160 * 2,
											144 * 2,
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
	mmu.register_chunk(0xFF40, 0xFF4B, &gpu); //gpu control regs
	mmu.register_chunk(0xFF80, 0xFFFE, &ram); //high ram
	mmu.register_chunk(0xFFFF, 0xFFFF, &ints); //interrupts

	gpu.ram_ptr = ram.get(); //TODO: REMOVE IT IMMEDIATELY!

	cpu.reset();
	cpu.attach_memory(&mmu);
	cpu.fill_tabs();

	bool spin = true;

	while (spin)
	{
		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
				case SDL_KEYDOWN:
					switch (ev.key.keysym.sym)
					{
						case SDLK_LEFT:
							joypad.push_key(K_LEFT);
							break;

						case SDLK_RIGHT:
							joypad.push_key(K_RIGHT);
							break;

						case SDLK_UP:
							joypad.push_key(K_UP);
							break;

						case SDLK_DOWN:
							joypad.push_key(K_DOWN);
							break;

						case SDLK_a:
							joypad.push_key(K_A);
							break;

						case SDLK_b:
							joypad.push_key(K_B);
							break;

						case SDLK_KP_ENTER:
							joypad.push_key(K_SELECT);
							break;

						case SDLK_s:
							joypad.push_key(K_START);
							break;
					}
					break;

				case SDL_KEYUP:
					switch (ev.key.keysym.sym)
					{
					case SDLK_LEFT:
						joypad.release_key(K_LEFT);
						break;

					case SDLK_RIGHT:
						joypad.release_key(K_RIGHT);
						break;

					case SDLK_UP:
						joypad.release_key(K_UP);
						break;

					case SDLK_DOWN:
						joypad.release_key(K_DOWN);
						break;

					case SDLK_a:
						joypad.release_key(K_A);
						break;

					case SDLK_b:
						joypad.release_key(K_B);
						break;

					case SDLK_KP_ENTER:
						joypad.release_key(K_SELECT);
						break;

					case SDLK_s:
						joypad.release_key(K_START);
						break;
					}
					break;
			}

			if (ev.type == SDL_QUIT)
				spin = false;
		}

		while (!gpu.is_entering_vblank()) //if someone turn off lcd, this loop may spin forever
		{
			u32 sync_cycles = 0;

			if (ints.is_any_raised())
			{
				cpu.unhalt();

				if (cpu.is_interrupt_enabled())
					sync_cycles = cpu.handle_interrupt(ints.get_first_raised());
			}

			sync_cycles += cpu.step();
			sync_cycles += gpu.step(sync_cycles, ints);

			if (timer.step(sync_cycles))
				ints.raise(INT_TIMER);
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
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);

	return 0;
}

//int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
//{
//	Cartrige cart;
//	Interrupts interrupts;
//	Ram ram;
//	MMU virtual_memory;
//	CPU cpu;
//	Gpu gpu;
//	Timer timer;
//	Joypad joypad;
//
//	virtual_memory.register_chunk(0, 0x7FFF, cart.get_memory_interface()); //cartrige rom banks
//	virtual_memory.register_chunk(0x8000, 0x9FFF, &gpu); //vram memory
//	virtual_memory.register_chunk(0xA000, 0xBFFF, cart.get_memory_interface()); //external ram
//	virtual_memory.register_chunk(0xC000, 0xFDFF, &ram); //internal ram
//	virtual_memory.register_chunk(0xFE00, 0xFE9F, &gpu); //oam tables
//	//virtual_memory.register_chunk(0xFEA0, 0xFEFF, &not_usable); //reserved memory
//	virtual_memory.register_chunk(0xFF00, 0xFF00, &joypad);//input keys register
//	//FF01-FF02 serial data transfer registers
//	virtual_memory.register_chunk(0xFF04, 0xFF07, &timer);//timer controls
//	virtual_memory.register_chunk(0xFF0F, 0xFF0F, &interrupts);//interrupts flags
//	//FF10 - FF3F should be audio registers
//	virtual_memory.register_chunk(0xFF40, 0xFF4B, &gpu); //gpu control regs
//	//FF4D - speed switch (CGB only)
//	virtual_memory.register_chunk(0xFF4F, 0xFF4F, &gpu); //gpu vram bank (GBC)
//	virtual_memory.register_chunk(0xFF51, 0xFF55, &gpu); //gpu hdma transfer (GBC)
//	//FF56 - led (CBG only)
//	virtual_memory.register_chunk(0xFF68, 0xFF6B, &gpu); //gpu GBC regs (GBC)
//	virtual_memory.register_chunk(0xFF70, 0xFF70, &ram); //internal RAM switch (GBC)
//	virtual_memory.register_chunk(0xFF80, 0xFFFE, &ram); //high ram
//	virtual_memory.register_chunk(0xFFFF, 0xFFFF, &interrupts); //interrupts
//
//	cpu.attach_memory(&virtual_memory);
//	cpu.fill_tabs();
//
//	bool double_speed = false;
//
//	while (wnd.end())
//	{
//		u32 cycles = 0;
//		const u32 clock_mul = double_speed ? 1 : 0; //multiply by 2, or 1
//		const u32 next_vblank = 70224 << clock_mul;
//
//		while (cycles < next_vblank)
//		{
//			u32 sync_cycles = 0;
//
//			if (interrupts.is_any_raised() && cpu.is_interrupt_enabled())
//				sync_cycles = cpu.handle_interrupt(interrupts.get_first_raised());
//
//			sync_cycles += cpu.step();
//			sync_cycles += gpu.step(sync_cycles, interrupts) << clock_mul; //if hdma is active, cpu is blocked
//			//apu.step(sync_cycles);
//			
//			if (timer.step(sync_cycles))
//				interrupts.raise(INT_TIMER);
//
//			cycles += sync_cycles;
//		}
//
//		renderer.draw(gpu.get_framebuffer());
//	}
//
//	return 0;
//}