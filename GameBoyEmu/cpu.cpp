#pragma once
#include "stdafx.h"
#include "cpu.h"

CPU::CPU(MMU& memory_controller) : mmu(memory_controller) 
{
	fill_instruction_maps(); 
	reset();
}

void CPU::reset()
{
	is_halted = false;
	interrupts = false;
	delayed_ei = false;
	cgb_mode = false;
	
	reg_16[AF] = 0x01B0;
	reg_16[BC] = 0x0013;
	reg_16[DE] = 0x00D8;
	reg_16[HL] = 0x014D;
	reg_16[SP] = 0xFFFE;
	pc = 0x100;
}

u32 CPU::step()
{
	if (is_halted)
		return 4;

	if (delayed_ei)
	{
		interrupts = true;
		delayed_ei = false;
	}

	u32 cycles_passed = 0;
	u8 opcode = fetch8(0);

	if (opcode != 0xCB)
		cycles_passed = (this->*instr_map[opcode])(opcode);

	else
	{
		u8 ext_opcode = fetch8(4);
		cycles_passed = (this->*ext_instr_map[ext_opcode])(ext_opcode);
	}

	return cycles_passed;
}

bool CPU::is_interrupt_enabled() const
{
	return interrupts;
}

void CPU::unhalt()
{
	is_halted = false;
}