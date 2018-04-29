#pragma once

#include "IMemory.h"

enum KEYS {K_RIGHT, K_LEFT, K_UP, K_DOWN, K_A, K_B, K_SELECT, K_START};

class Joypad final : public IMemory
{
	private:
		bool keys[8]; //switch all of this into one u8 + bitops
		bool dir_keys;
		bool sel_keys;

	public:
		Joypad() { for (auto& i : keys) i = true; dir_keys = false; sel_keys = false; }

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void push_key(KEYS keycode);
		void release_key(KEYS keycode);

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};