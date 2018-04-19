#pragma once
#include "stdafx.h"
#include <SDL.h>

#include "core/joypad.h"

class GuiSDL
{
	private:
		std::unordered_map<SDL_Keycode, KEYS> key_map;
		SDL_Window* window = nullptr;

	public:
		GuiSDL(u32 w, u32 h, const std::string& title);
		~GuiSDL();

		void set_window_title(const std::string& new_title);
		void* get_display() const;

		bool input_handler(Joypad& input);
};
