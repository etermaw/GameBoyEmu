#pragma once

#include <SDL.h>
#include "gui.h"

class RendererSDL
{
	private:
		std::unique_ptr<u32[]> frame_buffer;
		SDL_Window* window = nullptr;
		SDL_Renderer* rend = nullptr;
		SDL_Texture* tex = nullptr;

	public:
		RendererSDL(void* display);
		~RendererSDL();

		void vblank_handler(const u32* buffer);
		u32* draw_frame();
};