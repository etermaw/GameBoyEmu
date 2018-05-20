#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "utils/sha256.h"

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

	
	frame_buffer = std::make_unique<u32[]>(144*160);
	internal_buffer = std::make_unique<u8[]>(4 * (1 << 15));

	u8* ptr = internal_buffer.get();

	for (int i = 0; i < 4; ++i)
		dummy_buffers[i] = &ptr[i * (1 << 15)];

	frame_buffer_ptr = frame_buffer.get();
}

u32* Tester::draw_frame()
{
	if (calculate_hash)
	{
		calculate_hash = false;
		auto hash = sha256(reinterpret_cast<const u8*>(frame_buffer_ptr), sizeof(u32) * 160 * 144);

		for (auto i : hash)
			printf("%08x", i);

		fflush(stdout);
	}

	if (use_renderer)
		frame_buffer_ptr = render_callback();

	return frame_buffer_ptr;
}

bool Tester::is_running() const
{
	return running;
}

void Tester::pump_input(Core& emu_core)
{
	char buffer[1] = {}; //TODO: handle more messages per call
	int ret = read(0, buffer, 1);

	if (ret == -1)
	{
		if (!(errno == EAGAIN || errno == EWOULDBLOCK))
			running = false;
	}

	else
	{
		if (ret == 0 || buffer[0] == TC_EXIT)
			running = false; //TODO: print into stderr

		if (ret == 1)
		{
			switch (buffer[0])
			{
				case TC_CALCULATE_HASH:
					calculate_hash = true;
					break;

				case TC_PUSH_ALL_KEYS:
					for (u32 i = 0; i < 8; ++i)
						emu_core.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ALL_KEYS:
					for (u32 i = 0; i < 8; ++i)
						emu_core.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ARROWS:
					for (u32 i = 0; i < 4; ++i)
						emu_core.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ARROWS:
					for (u32 i = 0; i < 4; ++i)
						emu_core.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_ABSS:
					for (u32 i = 4; i < 8; ++i)
						emu_core.push_key(static_cast<KEYS>(i));
					break;


				case TC_RELEASE_ABSS:
					for (u32 i = 4; i < 8; ++i)
						emu_core.release_key(static_cast<KEYS>(i));
					break;


				case TC_PUSH_A:
				case TC_PUSH_B:
				case TC_PUSH_START:
				case TC_PUSH_SELECT:
				case TC_PUSH_UP:
				case TC_PUSH_DOWN:
				case TC_PUSH_LEFT:
				case TC_PUSH_RIGHT:
					emu_core.push_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;

				case TC_RELEASE_A:
				case TC_RELEASE_B:
				case TC_RELEASE_START:
				case TC_RELEASE_SELECT:
				case TC_RELEASE_UP:
				case TC_RELEASE_DOWN:
				case TC_RELEASE_LEFT:
				case TC_RELEASE_RIGHT:
					emu_core.release_key(static_cast<KEYS>(buffer[0] - TC_PUSH_RIGHT));
					break;
			}
		}
	}
}

void Tester::attach_renderer(function<u32*()> callback)
{
	render_callback = callback;
	use_renderer = true;
	frame_buffer_ptr = render_callback(); //override default buffer
}

void Tester::dummy(bool unused)
{
	UNUSED(unused);
}

u8** Tester::swap_buffers(u8** ptr, u32 unused)
{
	UNUSED(unused);
	return dummy_buffers;
}
