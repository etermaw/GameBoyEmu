#include "audio_postprocess.h"
#include <SDL.h>

Audio::Audio()
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

Audio::~Audio()
{
	SDL_CloseAudio();
}

u8** Audio::swap_buffers(u8** buffs, u32 count)
{
	if (buffs == nullptr)
		return buffers;

	float buf[2048];
    const u32 sample_count = (count + offset) / 48;

	for (size_t i = 0; i < sample_count; i++)
	{
        const u32 index = i * 48 + offset;
		u8 total = buffers[0][index] + buffers[1][index] + buffers[2][index] + buffers[3][index];
		buf[i * 2] = total * 0.025f;
		buf[i * 2 + 1] = total * 0.025f;
	}

    offset = (count + offset) % 48;

	SDL_QueueAudio(1, buf, sample_count * 2 * sizeof(float));

	while (SDL_GetQueuedAudioSize(1) > SAMPLE_COUNT * 2 * sizeof(float))
		SDL_Delay(1);

	return buffs;
}
