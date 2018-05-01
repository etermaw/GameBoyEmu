
#include "cpu.h"

CPU::CPU(MMU& memory_controller, Interrupts& ints) : mmu(memory_controller), ints(ints)
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
	
    //reg_16[AF] = 0x01B0; //DMG
	//reg_16[BC] = 0x0013;
	//reg_16[DE] = 0x00D8;
	//reg_16[HL] = 0x014D;

	reg_16[AF] = 0x1180; //CGB
	reg_16[BC] = 0x0000;
	reg_16[DE] = 0x0008;
	reg_16[HL] = 0x007C;
    
	reg_16[SP] = 0xFFFE;
	pc = 0x100;

	cycles_ahead = 0;
}

u32 CPU::handle_interrupt()
{
	interrupts = false;
	mmu.write_byte(--reg_16[SP], (pc >> 8) & 0xFF, 4);

	//advance all things by 8 cycles
	//TODO: watch out for dead code elimination!
	//TODO: try to find out better alternative to this (ucode?)
	mmu.read_byte(0x8000, 8); //GPU
	mmu.read_byte(0xFF04, 8); //timer
	mmu.read_byte(0xFF10, 8); //APU

	INTERRUPTS code = ints.get_first_raised();

	mmu.write_byte(--reg_16[SP], pc & 0xFF, 8);
	pc = ((code != INT_NONE) ? (0x40 + code * 8) : 0);

	return 20;
}

u32 CPU::step(u32 cycles)
{
	if (is_halted)
		return 4;

	if (delayed_ei)
	{
		interrupts = true;
		delayed_ei = false;
	}

	cycles_ahead = cycles;
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

void CPU::serialize(std::ostream& save_stream)
{
	save_stream.write(reinterpret_cast<char*>(reg_16), sizeof(u16) * REGISTER_16::R16_SIZE);
	save_stream << pc << interrupts << is_halted << delayed_ei << cgb_mode;
}

void CPU::deserialize(std::istream& load_stream)
{
	load_stream.read(reinterpret_cast<char*>(reg_16), sizeof(u16) * REGISTER_16::R16_SIZE);
	load_stream >> pc >> interrupts >> is_halted >> delayed_ei >> cgb_mode;
}

void CPU::get_cpu_state(std::array<u16, 5>& regs, bool& ints)
{
	std::memcpy(regs.data(), reg_16, sizeof(u16) * 5);
	ints = interrupts;
}

void CPU::attach_debugger(std::tuple<u16**, function<void(std::array<u16, 5>&, bool&)>*> params)
{
	*std::get<0>(params) = &pc;
	*std::get<1>(params) = make_function(&CPU::get_cpu_state, this);
}
