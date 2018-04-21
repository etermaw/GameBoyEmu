#pragma once
#include "stdafx.h"
#include <SDL.h>

#include "core/core.h"
#include "core/joypad.h"

class GuiSDL
{
	private:
		std::unordered_map<SDL_Keycode, KEYS> key_map;
		SDL_Window* window = nullptr;
		bool running = true;

	public:
		GuiSDL(u32 w, u32 h, const std::string& title);
		~GuiSDL();

		void set_window_title(const std::string& new_title);
		void* get_display() const;

		void pump_input(Core& emu_core);
		bool is_running() const;

		bool input_handler(Joypad& input);
};
