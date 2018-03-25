#include "stdafx.h"
#include "core/core.h"
#include "platform/dispatch.h"

int main(int argc, char *argv[])
{
	Platform::init();
	std::string file_name;

	auto ram_saver = [&](const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_ram", std::ios_base::trunc);
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	auto rtc_saver = [&](std::chrono::seconds epoch, const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_rtc", std::ios_base::trunc);
		to_save << epoch.count();
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	Core emu_core;
	Platform::Audio audio_post;
	Platform::Gui gui(3*160, 3*140, "GBE");
	Platform::Renderer renderer(gui.get_display());

	external_callbacks endpoints;

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&Platform::Audio::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&Platform::Audio::swap_buffers, &audio_post);
	endpoints.update_input = make_function(&Platform::Gui::input_handler, &gui);
	endpoints.draw_frame = make_function(&Platform::Renderer::vblank_handler, &renderer);

	emu_core.attach_callbacks(endpoints);

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		std::ifstream rom(file_name);

		if (rom.is_open())
		{
			std::ifstream ram(file_name + "_ram"), rtc(file_name + "_rtc");

			emu_core.load_cartrige(rom, ram, rtc);
			std::string cart_name = emu_core.get_cart_name();
			gui.set_window_title(cart_name);

			std::cout << "Cartrige loaded!\n";
			break;
		}

		std::cout << "Failed to load cartrige!\n";
	}

	emu_core.run();
	return 0;
}
