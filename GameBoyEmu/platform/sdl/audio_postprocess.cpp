#include "audio_postprocess.h"
#include <SDL.h>

AudioSDL::AudioSDL()
{
    internal_memory = std::make_unique<u8[]>(BUFFER_SIZE * 4);

	for (u32 i = 0; i < 4; ++i)
		buffers[i] = &internal_memory[BUFFER_SIZE * i];

	SDL_AudioSpec opt = {}, tmp = {};

	opt.freq = 44100;
	opt.channels = 2;
	opt.format = AUDIO_F32SYS;
	opt.samples = 1024;

	SDL_OpenAudio(&opt, &tmp);
	SDL_PauseAudio(0);
}

AudioSDL::~AudioSDL()
{
	SDL_CloseAudio();
}

u8** AudioSDL::swap_buffers(u8** buffs, u32 count)
{
	if (buffs == nullptr)
		return buffers;

	float buf[2048];
    const u32 sample_count = (count + offset) / 48;

	for (u32 i = 0; i < sample_count; i++)
	{
		//mix 4 samples and normalize (audio: from -1 to 1)
        const u32 index = i * 48 + offset;
		u8 total = buffers[0][index] + buffers[1][index] + buffers[2][index] + buffers[3][index];
		float in = (total * (1.0f / 30.0f)) - 1.0f;

		//apply high-pass filter
		xv[0] = xv[1];
		xv[1] = xv[2];
        xv[2] = in * (1.0f / 1.005049991f);

        yv[0] = yv[1]; yv[1] = yv[2]; 
        yv[2] = (xv[0] + xv[2]) - (2 * xv[1]) + (-0.9899760140f * yv[0]) + (1.9899255201f * yv[1]);
        float out = yv[2];

		buf[i * 2] = out;
		buf[i * 2 + 1] = out;
	}

	for (u32 i = 0; i < sample_count; ++i)
	{
		
	}

    offset = (count + offset) % 48;

	SDL_QueueAudio(1, buf, sample_count * 2 * sizeof(float));

	while (SDL_GetQueuedAudioSize(1) > SAMPLE_COUNT * 2 * sizeof(float))
		SDL_Delay(1);

	return buffs;
}
