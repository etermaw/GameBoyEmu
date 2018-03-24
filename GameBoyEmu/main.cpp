#include "stdafx.h"
#include "core/core.h"
#include "platform/dispatch.h"


#ifdef ENABLE_AUTO_TESTS
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "sha256.h"
bool calculate_screen_hash = false;

enum TEST_COMMANDS { TC_EXIT = 0, TC_CALCULATE_HASH, TC_PUSH_ALL_KEYS, TC_RELEASE_ALL_KEYS, 
					TC_PUSH_ARROWS, TC_RELEASE_ARROWS, TC_PUSH_ABSS, TC_RELEASE_ABSS, 
					TC_PUSH_RIGHT,TC_PUSH_LEFT,TC_PUSH_UP,TC_PUSH_DOWN,TC_PUSH_A,TC_PUSH_B,TC_PUSH_SELECT,TC_PUSH_START,
					TC_RELEASE_RIGHT,TC_RELEASE_LEFT,TC_RELEASE_UP,TC_RELEASE_DOWN,TC_RELEASE_A,TC_RELEASE_B,TC_RELEASE_SELECT,TC_RELEASE_START };

bool input_handler(Joypad& input)
{
	char buffer[1] = {};
	int ret = read(0, buffer, 1);

	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;

		else
			return false;
	}

	else
	{
		if (ret == 0 || buffer[0] == TC_EXIT)
			return false; //TODO: print into stderr

		if (ret == 1)
		{
			switch(buffer[0])
			{
				case TC_CALCULATE_HASH:
					calculate_screen_hash = true;
					break;

				case TC_PUSH_ALL_KEYS:
					for(u32 i = 0; i < 8; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ALL_KEYS:
					for(u32 i = 0; i < 8; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ARROWS:
					for(u32 i = 0; i < 4; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ARROWS:
					for(u32 i = 0; i < 4; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ABSS:
					for(u32 i = 4; i < 8; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ABSS:
					for(u32 i = 4; i < 8; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_A:
				case TC_PUSH_B:
				case TC_PUSH_START:
				case TC_PUSH_SELECT:
				case TC_PUSH_UP:
				case TC_PUSH_DOWN:
				case TC_PUSH_LEFT:
				case TC_PUSH_RIGHT:
					input.push_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;

				case TC_RELEASE_A:
				case TC_RELEASE_B:
				case TC_RELEASE_START:
				case TC_RELEASE_SELECT:
				case TC_RELEASE_UP:
				case TC_RELEASE_DOWN:
				case TC_RELEASE_LEFT:
				case TC_RELEASE_RIGHT:
					input.release_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;
			}
		}
		return true;
	}
}
#endif

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

#ifdef ENABLE_AUTO_TESTS
        if (calculate_screen_hash)
        {
            calculate_screen_hash = false;
            auto hash = sha256(reinterpret_cast<const u8*>(frame_buffer), sizeof(u32) * 160 * 144);

            for (auto i : hash)
                printf("%08x", i);

			fflush(stdout);
        }
#endif

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

#ifndef ENABLE_AUTO_TESTS
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
#else
    std::ifstream rom(argv[1]);

	if (rom.is_open())
	{
        std::ifstream ram(file_name + "_ram"), rtc(file_name + "_rtc");
            
		emu_core.load_cartrige(rom, ram, rtc);
        std::string cart_name = emu_core.get_cart_name();
        SDL_SetWindowTitle(window, cart_name.c_str());

		int flags1 = fcntl(STDOUT_FILENO, F_GETFL);
		int flags2 = fcntl(STDIN_FILENO, F_GETFL);

  		if (flags1 != -1)
    		fcntl(STDOUT_FILENO, F_SETFL, flags1 | O_NONBLOCK);

		if (flags2 != -1)
    		fcntl(STDIN_FILENO, F_SETFL, flags2 | O_NONBLOCK);
	}
#endif

	emu_core.run();
	return 0;
}
