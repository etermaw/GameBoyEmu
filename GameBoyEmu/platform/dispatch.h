#pragma once

#ifndef ENABLE_AUTO_TESTS

#include <SDL.h>
#include "sdl/audio_postprocess.h"
#include "sdl/renderer.h"
#include "sdl/gui.h"

namespace Platform {
	using Audio = AudioSDL;
	using Renderer = RendererSDL;
	using Gui = GuiSDL;

	inline void init() { SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); }
	inline void after_attach(Audio& a, Renderer& r, Gui& g) {}
}

#else

#include "linux/test.h"

namespace Platform {
	using Audio = AudioTEST;
	using Renderer = RendererTEST;
	using Gui = GuiTEST;

	inline void init() {}
	inline void after_attach(Audio& a, Renderer& r, Gui& g)
	{
		auto impl = std::make_shared<Tester>();

		a.attach_impl(impl);
		r.attach_impl(impl);
		g.attach_impl(impl);
	}
}

#endif
