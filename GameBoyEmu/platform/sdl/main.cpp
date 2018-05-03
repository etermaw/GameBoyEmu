#include "core/core.h"
#include "platform/dispatch.h"

int main(int argc, char *argv[])
{
	Platform::init();
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

	Platform::Audio audio_post;
	Platform::Gui gui(3*160, 3*144, "GBE");
	Platform::Renderer renderer(gui.get_display());

	external_callbacks endpoints;

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&Platform::Audio::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&Platform::Audio::swap_buffers, &audio_post);
	endpoints.draw_frame = make_function(&Platform::Renderer::vblank_handler, &renderer);

	emu_core.attach_callbacks(endpoints);

	Platform::after_attach(audio_post, renderer, gui);

#ifndef ENABLE_AUTO_TESTS
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

#else
	std::ifstream rom(argv[1], std::ios::binary);

	if (rom.is_open())
	{
		std::ifstream ram(file_name + "_ram", std::ios::binary), rtc(file_name + "_rtc", std::ios::binary);
		emu_core.load_cartrige(rom, ram, rtc);
	}
#endif

	while (gui.is_running())
	{
		gui.pump_input(emu_core);
		emu_core.run_one_frame();
	}

	return 0;
}