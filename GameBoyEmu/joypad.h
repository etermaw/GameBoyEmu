#pragma once
#include "stdafx.h"
#include "IMemory.h"

enum KEYS {K_RIGHT, K_LEFT, K_UP, K_DOWN, K_A, K_B, K_SELECT, K_START};

class Joypad final : public IMemory
{
	private:
		bool keys[8];
		bool dir_keys;

	public:
		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;

		void push_key(KEYS keycode);
		void release_key(KEYS keycode);
};