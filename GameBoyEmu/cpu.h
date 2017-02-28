#pragma once
#include "stdafx.h"
#include "cpu_instructions.h"
#include "cpu_debugger.h"

class CPU : public CPUCore
{
	friend class CPU_Debbuger;
	using instr = u32(CPUCore::*)(u8);

	private:	
		instr instruction_map[256];
		instr ext_instruction_map[256];

	public:
		void fill_tabs();
		void attach_memory(MMU* memory_controller);

		void reset();
		u32 step();
		u32 handle_interrupt(INTERRUPTS code); //it will cost 16~20 cycles!
		bool is_interrupt_enabled() const;
		void unhalt();
};