#include "stdafx.h"
#include "core.h"
#include "audio_postprocess.h"

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_Window* window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,	160 * 3, 144 * 3, 0);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	auto vblank_handler = [&](const u32* frame_buffer) 
	{
		void* pixels = nullptr;
		int pitch = 0;

		SDL_LockTexture(tex, NULL, &pixels, &pitch);
		std::memcpy(pixels, frame_buffer, sizeof(u32) * 160 * 144);
		SDL_UnlockTexture(tex);

		SDL_RenderClear(rend);
		SDL_RenderCopy(rend, tex, NULL, NULL);
		SDL_RenderPresent(rend); 
	};

	auto input_handler = [](Joypad& input)
	{
		SDL_Event ev;
		static const u32 key_map[8] = { SDLK_RIGHT, SDLK_LEFT, SDLK_UP, SDLK_DOWN, SDLK_a, SDLK_b, SDLK_RETURN, SDLK_s };

		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
			{
				auto key_code = std::find(std::begin(key_map), std::end(key_map), ev.key.keysym.sym);

				if (key_code != std::end(key_map))
				{
					auto index = std::distance(std::begin(key_map), key_code);

					if (ev.type == SDL_KEYDOWN)
						input.push_key(static_cast<KEYS>(index));

					else
						input.release_key(static_cast<KEYS>(index));
				}
			}

			else if (ev.type == SDL_QUIT)
				return false;
		}

		return true;
	};

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
	Audio audio_post;

	external_callbacks endpoints;

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&Audio::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&Audio::swap_buffers, &audio_post);
	endpoints.update_input = function<bool(Joypad&)>(input_handler);
	endpoints.draw_frame = function<void(const u32*)>(vblank_handler);

	emu_core.attach_callbacks(endpoints);

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		std::ifstream rom(file_name);

		if (rom.is_open())
		{
			emu_core.load_cartrige(rom, std::ifstream(file_name + "_ram"), std::ifstream(file_name + "_rtc"));
			std::cout << "Cartrige loaded!\n";
			break;
		}

		std::cout << "Failed to load cartrige!\n";
	}

	emu_core.run();

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);

	return 0;
}
