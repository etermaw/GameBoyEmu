#pragma once

#include <SDL.h>
#include "sdl/audio_postprocess.h"
#include "sdl/renderer.h"
#include "sdl/gui.h"

namespace Platform {
	using Audio = AudioSDL;
	using Renderer = RendererSDL;
	using Gui = GuiSDL;

	inline void init() { SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); }
};