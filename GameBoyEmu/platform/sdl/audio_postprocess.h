#pragma once


class AudioSDL
{
	static const u32 SAMPLE_COUNT = 512;
	static const u32 BUFFER_SIZE = 1 << 15;

	private:
        std::unique_ptr<u8[]> internal_memory;
		u8* buffers[4];
		float xv[3] = {};
		float yv[3] = {};
        u32 offset = 0;

	public:
		AudioSDL();
		~AudioSDL();

		u8** swap_buffers(u8** buffers, u32 count);
		void dummy(bool) {}
};
