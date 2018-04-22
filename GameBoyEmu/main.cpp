#include "stdafx.h"
#include "core/core.h"
#include "platform/dispatch.h"

#ifdef ENABLE_AUTO_TESTS
#include "platform/linux/test.h"
#endif

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

#ifndef ENABLE_AUTO_TESTS
	Platform::Audio audio_post;
	Platform::Gui gui(3*160, 3*144, "GBE");
	Platform::Renderer renderer(gui.get_display());

	external_callbacks endpoints;

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&Platform::Audio::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&Platform::Audio::swap_buffers, &audio_post);
	endpoints.draw_frame = make_function(&Platform::Renderer::vblank_handler, &renderer);
#else
	Platform::Gui gui(3*160, 3*144, "GBE");
	Platform::Renderer renderer(gui.get_display());
	Tester tester;
	external_callbacks endpoints;

	tester.attach_renderer(make_function(&Platform::Renderer::vblank_handler, &renderer));

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&Tester::audio_dummy_ctrl, &tester);
	endpoints.swap_sample_buffer = make_function(&Tester::audio_dummy_swap, &tester);
	endpoints.draw_frame = make_function(&Tester::render_stub, &tester);
#endif

	emu_core.attach_callbacks(endpoints);

#ifndef ENABLE_AUTO_TESTS
	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		std::ifstream rom(file_name, std::ios::binary | std::ios::in);

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

#else
	std::ifstream rom(argv[1]);

	if (rom.is_open())
	{
		std::ifstream ram(file_name + "_ram"), rtc(file_name + "_rtc");
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
