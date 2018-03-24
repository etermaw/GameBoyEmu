#pragma once

#include "sdl/audio_postprocess.h"
#include "sdl/renderer.h"
#include "sdl/gui.h"

namespace Platform {
	using Audio = AudioSDL;
	using Renderer = RendererSDL;
	using Gui = GuiSDL;
};