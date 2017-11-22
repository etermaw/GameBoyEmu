#include "audio_postprocess.h"

Audio::Audio()
{
	constexpr double ratio = (1 << 21) / 44100.0;
	constexpr u32 t = static_cast<int>(ratio + 0.5);
	sample_threshold = t * 512;

	for (auto& i : buffers)
		i = new u8[BUFFER_SIZE];

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
	for (auto& i : buffers)
		delete[] i;

	SDL_CloseAudio();
}

u8** Audio::swap_buffers(u8** buffs, u32 count)
{
	if (buffs == nullptr)
		return buffers;

	float buf[SAMPLE_COUNT * 2];

	for (size_t i = 0; i < SAMPLE_COUNT; i++)
	{
		u8 total = buffers[0][i * 48] + buffers[1][i * 48] + buffers[2][i * 48] + buffers[3][i * 48];
		buf[i * 2] = total * 0.05f;
		buf[i * 2 + 1] = total * 0.05f;
	}

	SDL_QueueAudio(1, buf, SAMPLE_COUNT * 2 * sizeof(float));

	while (SDL_GetQueuedAudioSize(1) > SAMPLE_COUNT * 2 * sizeof(float))
		SDL_Delay(1);

	return buffs;
}
