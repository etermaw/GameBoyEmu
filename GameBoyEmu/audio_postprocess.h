#pragma once
#include "stdafx.h"

class Audio
{
	static const u32 SAMPLE_COUNT = 512;
	static const u32 BUFFER_SIZE = 1 << 15;

	private:
		u8* buffers[4];
		u32 sample_threshold;

	public:
		Audio();
		~Audio();

		u8** swap_buffers(u8** buffers, u32 count);
		void dummy(bool) {}
};