#pragma once
#include "stdafx.h"
#include "mmu.h"

//TODO those enums must be private!
enum FLAGS { F_C = 4, F_H, F_N, F_Z };
//enum REGISTER_8 { B, C, D, E, H, L, UNUSED_1, UNUSED_2, A, F, R8_SIZE }; //big endian 
enum REGISTER_8 { C, B, E, D, L, H, UNUSED_1, UNUSED_2, F, A, R8_SIZE }; //little endian
enum REGISTER_16 { BC, DE, HL, SP, AF, R16_SIZE };

template<class T>
class CPU
{
	protected:
		using op_fun = u32(T::*)(u8);

		op_fun instr_map[256];
		op_fun ext_instr_map[256];

		MMU* mmu;

		union
		{
			u16 reg_16[REGISTER_16::R16_SIZE];
			u8 reg_8[REGISTER_8::R8_SIZE];
		};
		
		u16 pc;

		bool interrupts;
		bool is_halted;

		void debug() {}; //dummy implementation

	public:
		CPU();
		void attach_memory(MMU* memory_controller);

		void reset();
		u32 step();
		u32 handle_interrupt(INTERRUPTS code); //it will cost 16~20 cycles!
		bool is_interrupt_enabled() const;
		void unhalt();
};

template<class T>
inline CPU<T>::CPU()
{
	static_cast<T*>(this)->fill_instruction_maps();
}

template<class T>
inline void CPU<T>::attach_memory(MMU* memory_controller)
{
	mmu = memory_controller;
}

template<class T>
inline void CPU<T>::reset()
{
	is_halted = false;
	interrupts = false;

	reg_16[AF] = 0x01B0;
	reg_16[BC] = 0x0013;
	reg_16[DE] = 0x00D8;
	reg_16[HL] = 0x014D;
	reg_16[SP] = 0xFFFE;
	pc = 0x100;
}

template<class T>
inline u32 CPU<T>::step()
{
	static_cast<T*>(this)->debug();

	if (is_halted)
		return 4;

	u32 cycles_passed = 0;
	u8 opcode = static_cast<T*>(this)->fetch_op();

	if (opcode != 0xCB)
		cycles_passed = (static_cast<T*>(this)->*instr_map[opcode])(opcode);

	else
	{
		u8 ext_opcode = static_cast<T*>(this)->fetch_ext_op();
		cycles_passed = (static_cast<T*>(this)->*ext_instr_map[ext_opcode])(ext_opcode);
	}

	return cycles_passed;
}

template<class T>
inline u32 CPU<T>::handle_interrupt(INTERRUPTS code)
{
	return static_cast<T*>(this)->interrupt_handler(code);
}

template<class T>
inline bool CPU<T>::is_interrupt_enabled() const
{
	return interrupts;
}

template<class T>
inline void CPU<T>::unhalt()
{
	is_halted = false;
}
