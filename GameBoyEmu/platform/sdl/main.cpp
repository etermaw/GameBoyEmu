#include <SDL.h>
#include "core/core.h"
#include "renderer.h"
#include "gui.h"
#include "audio_postprocess.h"

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	std::string file_name;

	auto ram_saver = [&](const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_ram", std::ios::trunc | std::ios::binary);
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

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

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&AudioSDL::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&AudioSDL::swap_buffers, &audio_post);

	emu_core.attach_callbacks(endpoints);
	emu_core.enable_debugger();

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		std::ifstream rom(file_name, std::ios::binary);

		if (rom.is_open())
		{
			std::ifstream ram(file_name + "_ram", std::ios::binary), rtc(file_name + "_rtc", std::ios::binary);

			emu_core.load_cartrige(rom, ram, rtc);
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

	return 0;
}
