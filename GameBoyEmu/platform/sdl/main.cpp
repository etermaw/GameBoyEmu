#include <SDL.h>
#include "platform/sdl/audio_postprocess.h"
#include "platform/sdl/renderer.h"
#include "platform/sdl/gui.h"
#include "core/core.h"

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	std::string file_name;

	auto rtc_saver = [&](std::chrono::seconds epoch, const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_rtc", std::ios::trunc | std::ios::binary);
		to_save << epoch.count();
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	Core emu_core;

	AudioSDL audio_post;
	GuiSDL gui(3*160, 3*144, "GBE");
	RendererSDL renderer(gui.get_display());

	external_callbacks endpoints;

	endpoints.audio_control = make_function(&AudioSDL::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&AudioSDL::swap_buffers, &audio_post);

	emu_core.attach_callbacks(endpoints);
	emu_core.enable_debugger();

	std::unique_ptr<u8[]> rom_mem;
	std::unique_ptr<u8[]> ram_mem;
	std::array<u8, 5> rtc_mem;

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		std::ifstream rom(file_name, std::ios::binary);

		if (rom.is_open())
		{
			//load rom
			rom.seekg(0, std::ios_base::end);
			const auto rom_size = rom.tellg();
			rom.seekg(0, std::ios_base::beg);

			rom_mem = std::make_unique<u8[]>(rom_size);
			rom.read(reinterpret_cast<char*>(rom_mem.get()), rom_size);

			//attach rom
			emu_core.load_rom(rom_mem.get(), rom_size);

			//if cartrige has ram, create it
			const u32 ram_size = emu_core.get_ram_size();

			if (ram_size > 0)
			{
				ram_mem = std::make_unique<u8[]>(ram_size);

				if (emu_core.has_battery_ram())
				{
					std::ifstream ram(file_name + "_ram", std::ios::binary);

					if (ram.is_open())
						ram.read(reinterpret_cast<char*>(ram_mem.get()), ram_size);
				}

				emu_core.load_ram(ram_mem.get(), ram_size);
			}

			if (emu_core.has_rtc())
			{
				std::ifstream rtc(file_name + "_rtc", std::ios::binary);

				if (rtc.is_open())
				{
					i64 timestamp = 0;
					rtc >> timestamp;
					rtc.read(reinterpret_cast<char*>(rtc_mem.data()), rtc_mem.size());

					//assumption: it`s UNIX time (until C++20 it`s implementation defined)
					const auto current_time = std::chrono::system_clock::now();
					const auto prev_time = std::chrono::time_point<std::chrono::system_clock>(std::chrono::seconds(timestamp));
					const auto passed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - prev_time);

					if (check_bit(rtc_mem[5], 6)) //if clock enabled
					{
						auto new_seconds = rtc_mem[0] + passed_time.count();
						auto new_minutes = rtc_mem[1] + new_seconds / 60;
						auto new_hours = rtc_mem[2] + new_minutes / 60;
						auto new_days = (((rtc_mem[4] & 1) << 8) | rtc_mem[3]) + (new_hours / 24);

						rtc_mem[0] = new_seconds % 60;
						rtc_mem[1] = new_minutes % 60;
						rtc_mem[2] = new_hours % 24;
						rtc_mem[3] = (new_days % 512) & 0xFF;
						rtc_mem[4] = change_bit(rtc_mem[4], (new_days % 512) > 255, 0);
						rtc_mem[4] = change_bit(rtc_mem[4], new_days > 511, 7);
					}
			
					//TODO: create local current_time for MBC
				}

				emu_core.load_rtc(rtc_mem.data(), rtc_mem.size());
			}

			emu_core.setup_core();

			std::string cart_name = emu_core.get_cart_name();
			gui.set_window_title(cart_name);

			std::cout << "Cartrige loaded!\n";
			break;
		}

		std::cout << "Failed to load cartrige!\n";
	}

	emu_core.set_frame_buffer(renderer.draw_frame());

	while (gui.is_running())
	{
		gui.pump_input(emu_core);
		emu_core.run_one_frame();
		emu_core.set_frame_buffer(renderer.draw_frame());
	}

	//save ram if it has battery
	if (emu_core.has_battery_ram())
	{
		const u32 ram_size = emu_core.get_ram_size();
		std::ofstream to_save(file_name + "_ram", std::ios::trunc | std::ios::binary);
		to_save.write(reinterpret_cast<const char*>(ram_mem.get()), ram_size);
	}

	//save rtc if present
	if (emu_core.has_rtc())
	{
		//TODO: if enabled, make last update using var given to mbc

		const auto new_timestamp = std::chrono::system_clock::now();

		std::ofstream to_save(file_name + "_rtc", std::ios::trunc | std::ios::binary);
		to_save << new_timestamp.time_since_epoch().count();
		to_save.write(reinterpret_cast<const char*>(rtc_mem.data()), rtc_mem.size());
	}

	return 0;
}
