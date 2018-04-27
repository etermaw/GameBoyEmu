#pragma once
//#include "stdafx.h"
#include <SDL.h>

#include "gui.h"

class RendererSDL
{
	private:
		SDL_Window* window = nullptr;
		SDL_Renderer* rend = nullptr;
		SDL_Texture* tex = nullptr;

	public:
		RendererSDL(void* display);
		~RendererSDL();

		void vblank_handler(const u32* buffer);
};