#include <SDL.h>
#include "renderer.h"

RendererSDL::RendererSDL(void* display)
{
	frame_buffer = std::make_unique<u32[]>(160*144);
	std::memset(frame_buffer.get(), 0xFF, sizeof(u32) * 144 * 160);

	SDL_Window* window = reinterpret_cast<SDL_Window*>(display);

	rend = SDL_CreateRenderer(window, -1, 0);
	tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
}

RendererSDL::~RendererSDL()
{
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(rend);
}

u32* RendererSDL::draw_frame()
{
	void* pixels = nullptr;
	int pitch = 0;

	SDL_LockTexture(tex, NULL, &pixels, &pitch);
	std::memcpy(pixels, frame_buffer.get(), sizeof(u32) * 160 * 144);
	SDL_UnlockTexture(tex);

	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);

	std::memset(frame_buffer.get(), 0xFF, sizeof(u32) * 144 * 160);
	return frame_buffer.get();
}
