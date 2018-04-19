#include "gui.h"

GuiSDL::GuiSDL(u32 w, u32 h, const std::string& title)
{
	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0);

	key_map[SDLK_RIGHT] = KEYS::K_RIGHT;
	key_map[SDLK_LEFT] = KEYS::K_LEFT;
	key_map[SDLK_UP] = KEYS::K_UP;
	key_map[SDLK_DOWN] = KEYS::K_DOWN;
	key_map[SDLK_a] = KEYS::K_A;
	key_map[SDLK_b] = KEYS::K_B;
	key_map[SDLK_RETURN] = KEYS::K_SELECT;
	key_map[SDLK_s] = KEYS::K_START;
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

	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP)
		{
			const auto key_code = key_map.find(ev.key.keysym.sym);

			if (key_code != key_map.end())
			{
				if (ev.type == SDL_KEYDOWN)
					input.push_key(key_code->second);

				else
					input.release_key(key_code->second);
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
