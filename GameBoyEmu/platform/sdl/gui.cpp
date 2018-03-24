#include "gui.h"

GuiSDL::GuiSDL(u32 w, u32 h, const std::string& title)
{
	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0);
}

GuiSDL::~GuiSDL()
{
	SDL_DestroyWindow(window);
}

void GuiSDL::set_window_title(const std::string& new_title)
{
	SDL_SetWindowTitle(window, new_title.c_str());
}

void* GuiSDL::get_display() const
{
	return window;
}

bool GuiSDL::input_handler(Joypad& input)
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

			else if (ev.key.keysym.sym == SDLK_ESCAPE)
			{
				if (ev.type == SDL_KEYDOWN)
				{
					input.push_key(K_A);
					input.push_key(K_B);
					input.push_key(K_SELECT);
					input.push_key(K_START);
				}

				else
				{
					input.release_key(K_A);
					input.release_key(K_B);
					input.release_key(K_SELECT);
					input.release_key(K_START);
				}
			}
		}

		else if (ev.type == SDL_QUIT)
			return false;
	}

	return true;
}
