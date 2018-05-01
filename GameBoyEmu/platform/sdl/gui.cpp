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

void GuiSDL::pump_input(Core& emu_core)
{
	SDL_Event ev;
	bool new_running = true;

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				const auto key_code = key_map.find(ev.key.keysym.sym);

				if (key_code != key_map.end())
				{
					if (ev.type == SDL_KEYDOWN)
						emu_core.push_key(key_code->second);

					else
						emu_core.release_key(key_code->second);
				}

				else if (ev.key.keysym.sym == SDLK_ESCAPE)
				{
					if (ev.type == SDL_KEYDOWN)
					{
						emu_core.push_key(K_A);
						emu_core.push_key(K_B);
						emu_core.push_key(K_SELECT);
						emu_core.push_key(K_START);
					}

					else
					{
						emu_core.release_key(K_A);
						emu_core.release_key(K_B);
						emu_core.release_key(K_SELECT);
						emu_core.release_key(K_START);
					}
				}

				break;
			}

			case SDL_QUIT:
				new_running = false;
				break;
		}
	}

	running = new_running;
}

bool GuiSDL::is_running() const
{
	return running;
}
