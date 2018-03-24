#include <SDL.h>
#include "renderer.h"

RendererSDL::RendererSDL(void* display)
{
	SDL_Window* window = reinterpret_cast<SDL_Window*>(display);

	rend = SDL_CreateRenderer(window, -1, 0);
	tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
}

RendererSDL::~RendererSDL()
{
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rend);
}

void RendererSDL::vblank_handler(const u32* frame_buffer)
{
	void* pixels = nullptr;
	int pitch = 0;

	SDL_LockTexture(tex, NULL, &pixels, &pitch);
	std::memcpy(pixels, frame_buffer, sizeof(u32) * 160 * 144);
	SDL_UnlockTexture(tex);

	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);
}
