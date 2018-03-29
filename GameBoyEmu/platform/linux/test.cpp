#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"

enum TEST_COMMANDS {
	TC_EXIT = 0, TC_CALCULATE_HASH, TC_PUSH_ALL_KEYS, TC_RELEASE_ALL_KEYS,
	TC_PUSH_ARROWS, TC_RELEASE_ARROWS, TC_PUSH_ABSS, TC_RELEASE_ABSS,
	TC_PUSH_RIGHT, TC_PUSH_LEFT, TC_PUSH_UP, TC_PUSH_DOWN, TC_PUSH_A, TC_PUSH_B, TC_PUSH_SELECT, TC_PUSH_START,
	TC_RELEASE_RIGHT, TC_RELEASE_LEFT, TC_RELEASE_UP, TC_RELEASE_DOWN, TC_RELEASE_A, TC_RELEASE_B, TC_RELEASE_SELECT, TC_RELEASE_START
};

Tester::Tester()
{
	//set-up non blocking I/O
	int flags1 = fcntl(STDOUT_FILENO, F_GETFL);
	int flags2 = fcntl(STDIN_FILENO, F_GETFL);

	if (flags1 != -1)
		fcntl(STDOUT_FILENO, F_SETFL, flags1 | O_NONBLOCK);

	if (flags2 != -1)
		fcntl(STDIN_FILENO, F_SETFL, flags2 | O_NONBLOCK);

	for (auto& i : dummy_ptrs)
		i = new u8[1<<15];
}

Tester::~Tester()
{
	for (auto& i : dummy_ptrs)
		delete[] i;
}

void Tester::render_stub(const u32* frame_buffer)
{
	if (calculate_hash)
	{
		calculate_hash = false;
		auto hash = sha256(reinterpret_cast<const u8*>(frame_buffer), sizeof(u32) * 160 * 144);

		for (auto i : hash)
			printf("%08x", i);

		fflush(stdout);
	}
}

bool Tester::input_stub(Joypad& input)
{
	char buffer[1] = {}; //TODO: handle more messages per call
	int ret = read(0, buffer, 1);

	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;

		else
			return false;
	}

	else
	{
		if (ret == 0 || buffer[0] == TC_EXIT)
			return false; //TODO: print into stderr

		if (ret == 1)
		{
			switch (buffer[0])
			{
				case TC_CALCULATE_HASH:
					calculate_hash = true;
					break;

				case TC_PUSH_ALL_KEYS:
					for (u32 i = 0; i < 8; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ALL_KEYS:
					for (u32 i = 0; i < 8; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ARROWS:
					for (u32 i = 0; i < 4; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ARROWS:
					for (u32 i = 0; i < 4; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ABSS:
					for (u32 i = 4; i < 8; ++i)
						input.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ABSS:
					for (u32 i = 4; i < 8; ++i)
						input.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_A:
				case TC_PUSH_B:
				case TC_PUSH_START:
				case TC_PUSH_SELECT:
				case TC_PUSH_UP:
				case TC_PUSH_DOWN:
				case TC_PUSH_LEFT:
				case TC_PUSH_RIGHT:
					input.push_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;

				case TC_RELEASE_A:
				case TC_RELEASE_B:
				case TC_RELEASE_START:
				case TC_RELEASE_SELECT:
				case TC_RELEASE_UP:
				case TC_RELEASE_DOWN:
				case TC_RELEASE_LEFT:
				case TC_RELEASE_RIGHT:
					input.release_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;
			}
		}
		return true;
	}
}

void Tester::audio_dummy_ctrl(bool unused)
{
	UNUSED(unused);
}

u8** Tester::audio_dummy_swap(u8** ptrs, u32 unused)
{
	UNUSED(ptrs);
	UNUSED(unused);

	return dummy_ptrs;
}