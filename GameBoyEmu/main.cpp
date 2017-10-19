#include "stdafx.h"
#include "Core.h"
#include "audio_postprocess.h"


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_Window* window = SDL_CreateWindow("Test",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		160 * 3,
		144 * 3,
		0);

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

	Core emu_core;
	Audio audio_post;

	external_callbacks endpoints;

	endpoints.audio_control = make_function(&Audio::dummy, &audio_post);
	endpoints.swap_sample_buffer = make_function(&Audio::swap_buffers, &audio_post);
	endpoints.draw_frame = make_function(&decltype(vblank_handler)::operator(), &vblank_handler);
	endpoints.update_input = make_function(&decltype(input_handler)::operator(), &input_handler);

	emu_core.attach_callbacks(endpoints);

	std::string file_name;

	while (true)
	{
		std::cout << "Insert cartrige path:\n";
		std::cin >> file_name;

		if (emu_core.load_cartrige(file_name))
		{
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
