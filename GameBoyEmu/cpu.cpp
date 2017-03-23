#include "cpu.h"

#define RANGE_FILL(tab, beg, end, f1, f2) do{\
											for(int i = beg; i < end; ++i)\
												{tab[i] = &CPUCore::f1;}\
												tab[beg + 6] = &CPUCore::f2;\
											} while(0)

void CPU::fill_tabs()
{
	for (auto& i : instruction_map)
		i = &CPUCore::illegal_op;

	for (auto& i : ext_instruction_map)
		i = &CPUCore::illegal_op;

	instruction_map[0x00] = &CPUCore::nop;
	instruction_map[0x01] = &CPUCore::ld_rr_nn;
	instruction_map[0x02] = &CPUCore::ld_rr_a;
	instruction_map[0x03] = &CPUCore::inc_rr;
	instruction_map[0x04] = &CPUCore::inc_r;
	instruction_map[0x05] = &CPUCore::dec_r;
	instruction_map[0x06] = &CPUCore::ld_r_n;
	instruction_map[0x07] = &CPUCore::rlca;
	instruction_map[0x08] = &CPUCore::ld_nn_sp;
	instruction_map[0x09] = &CPUCore::add_hl_rr;
	instruction_map[0x0A] = &CPUCore::ld_a_adr;
	instruction_map[0x0B] = &CPUCore::dec_rr;
	instruction_map[0x0C] = &CPUCore::inc_r;
	instruction_map[0x0D] = &CPUCore::dec_r;
	instruction_map[0x0E] = &CPUCore::ld_r_n;
	instruction_map[0x0F] = &CPUCore::rrca;

	instruction_map[0x10] = &CPUCore::stop;
	instruction_map[0x11] = &CPUCore::ld_rr_nn;
	instruction_map[0x12] = &CPUCore::ld_rr_a;
	instruction_map[0x13] = &CPUCore::inc_rr;
	instruction_map[0x14] = &CPUCore::inc_r;
	instruction_map[0x15] = &CPUCore::dec_r;
	instruction_map[0x16] = &CPUCore::ld_r_n;
	instruction_map[0x17] = &CPUCore::rla;
	instruction_map[0x18] = &CPUCore::jr_n;
	instruction_map[0x19] = &CPUCore::add_hl_rr;
	instruction_map[0x1A] = &CPUCore::ld_a_adr;
	instruction_map[0x1B] = &CPUCore::dec_rr;
	instruction_map[0x1C] = &CPUCore::inc_r;
	instruction_map[0x1D] = &CPUCore::dec_r;
	instruction_map[0x1E] = &CPUCore::ld_r_n;
	instruction_map[0x1F] = &CPUCore::rra;

	instruction_map[0x20] = &CPUCore::jr_cond_r;
	instruction_map[0x21] = &CPUCore::ld_rr_nn;
	instruction_map[0x22] = &CPUCore::ldi_hl_a;
	instruction_map[0x23] = &CPUCore::inc_rr;
	instruction_map[0x24] = &CPUCore::inc_r;
	instruction_map[0x25] = &CPUCore::dec_r;
	instruction_map[0x26] = &CPUCore::ld_r_n;
	instruction_map[0x27] = &CPUCore::daa;
	instruction_map[0x28] = &CPUCore::jr_cond_r;
	instruction_map[0x29] = &CPUCore::add_hl_rr;
	instruction_map[0x2A] = &CPUCore::ldi_a_hl;
	instruction_map[0x2B] = &CPUCore::dec_rr;
	instruction_map[0x2C] = &CPUCore::inc_r;
	instruction_map[0x2D] = &CPUCore::dec_r;
	instruction_map[0x2E] = &CPUCore::ld_r_n;
	instruction_map[0x2F] = &CPUCore::cpl;

	instruction_map[0x30] = &CPUCore::jr_cond_r;
	instruction_map[0x31] = &CPUCore::ld_rr_nn;
	instruction_map[0x32] = &CPUCore::ldd_hl_a;
	instruction_map[0x33] = &CPUCore::inc_rr;
	instruction_map[0x34] = &CPUCore::inc_hl;
	instruction_map[0x35] = &CPUCore::dec_hl;
	instruction_map[0x36] = &CPUCore::ld_hl_n;
	instruction_map[0x37] = &CPUCore::scf;
	instruction_map[0x38] = &CPUCore::jr_cond_r;
	instruction_map[0x39] = &CPUCore::add_hl_rr;
	instruction_map[0x3A] = &CPUCore::ldd_a_hl;
	instruction_map[0x3B] = &CPUCore::dec_rr;
	instruction_map[0x3C] = &CPUCore::inc_r;
	instruction_map[0x3D] = &CPUCore::dec_r;
	instruction_map[0x3E] = &CPUCore::ld_r_n;
	instruction_map[0x3F] = &CPUCore::ccf;

	for (int i = 0x40; i < 0x70; ++i)
		instruction_map[i] = &CPUCore::ld_r_r;

	for (int i = 0x46; i < 0x70; i += 8)
		instruction_map[i] = &CPUCore::ld_r_hl;

	RANGE_FILL(instruction_map, 0x70, 0x78, ld_hl_r, halt);
	RANGE_FILL(instruction_map, 0x78, 0x80, ld_r_r, ld_r_hl);
	RANGE_FILL(instruction_map, 0x80, 0x88, add_a_r, add_a_hl);
	RANGE_FILL(instruction_map, 0x88, 0x90, adc_a_r, adc_a_hl);
	RANGE_FILL(instruction_map, 0x90, 0x98, sub_a_r, sub_a_hl);
	RANGE_FILL(instruction_map, 0x98, 0xA0, sbc_a_r, sbc_a_hl);
	RANGE_FILL(instruction_map, 0xA0, 0xA8, and_a_r, and_a_hl);
	RANGE_FILL(instruction_map, 0xA8, 0xB0, xor_a_r, xor_a_hl);
	RANGE_FILL(instruction_map, 0xB0, 0xB8, or_a_r, or_a_hl);
	RANGE_FILL(instruction_map, 0xB8, 0xC0, cp_a_r, cp_a_hl);

	// C0
	instruction_map[0xC0] = &CPUCore::ret_cond;
	instruction_map[0xC1] = &CPUCore::pop_rr;
	instruction_map[0xC2] = &CPUCore::jp_cond_nn;
	instruction_map[0xC3] = &CPUCore::jp_nn;
	instruction_map[0xC4] = &CPUCore::call_cond_nn;
	instruction_map[0xC5] = &CPUCore::push_rr;
	instruction_map[0xC6] = &CPUCore::add_a_n;
	instruction_map[0xC7] = &CPUCore::rst_nn;
	instruction_map[0xC8] = &CPUCore::ret_cond;
	instruction_map[0xC9] = &CPUCore::ret;
	instruction_map[0xCA] = &CPUCore::jp_cond_nn;
	//instruction_map[0xCB] extension
	instruction_map[0xCC] = &CPUCore::call_cond_nn;
	instruction_map[0xCD] = &CPUCore::call_nn;
	instruction_map[0xCE] = &CPUCore::adc_a_n;
	instruction_map[0xCF] = &CPUCore::rst_nn;

	// D0
	instruction_map[0xD0] = &CPUCore::ret_cond;
	instruction_map[0xD1] = &CPUCore::pop_rr;
	instruction_map[0xD2] = &CPUCore::jp_cond_nn;
	//instruction_map[0xD3] null
	instruction_map[0xD4] = &CPUCore::call_cond_nn;
	instruction_map[0xD5] = &CPUCore::push_rr;
	instruction_map[0xD6] = &CPUCore::sub_a_n;
	instruction_map[0xD7] = &CPUCore::rst_nn;
	instruction_map[0xD8] = &CPUCore::ret_cond;
	instruction_map[0xD9] = &CPUCore::reti;
	instruction_map[0xDA] = &CPUCore::jp_cond_nn;
	//instruction_map[0xDB] null
	instruction_map[0xDC] = &CPUCore::call_cond_nn;
	//instruction_map[0xDD] null
	instruction_map[0xDE] = &CPUCore::sbc_a_n;
	instruction_map[0xDF] = &CPUCore::rst_nn;

	// E0
	instruction_map[0xE0] = &CPUCore::ldh_n_a;
	instruction_map[0xE1] = &CPUCore::pop_rr;
	instruction_map[0xE2] = &CPUCore::ldh_c_a;
	//instruction_map[0xE3] null
	//instruction_map[0xE4] null
	instruction_map[0xE5] = &CPUCore::push_rr;
	instruction_map[0xE6] = &CPUCore::and_n;
	instruction_map[0xE7] = &CPUCore::rst_nn;
	instruction_map[0xE8] = &CPUCore::add_sp_n;
	instruction_map[0xE9] = &CPUCore::jp_hl;
	instruction_map[0xEA] = &CPUCore::ld_nn_a;
	//instruction_map[0xEB] null
	//instruction_map[0xEC] null
	//instruction_map[0xED] null
	instruction_map[0xEE] = &CPUCore::xor_n;
	instruction_map[0xEF] = &CPUCore::rst_nn;

	// F0
	instruction_map[0xF0] = &CPUCore::ldh_a_n;
	instruction_map[0xF1] = &CPUCore::pop_af;
	instruction_map[0xF2] = &CPUCore::ldh_a_c;
	instruction_map[0xF3] = &CPUCore::di;
	//instruction_map[0xF4] null
	instruction_map[0xF5] = &CPUCore::push_af;
	instruction_map[0xF6] = &CPUCore::or_n;
	instruction_map[0xF7] = &CPUCore::rst_nn;
	instruction_map[0xF8] = &CPUCore::ld_hl_sp_n;
	instruction_map[0xF9] = &CPUCore::ld_sp_hl;
	instruction_map[0xFA] = &CPUCore::ld_a_nn;
	instruction_map[0xFB] = &CPUCore::ei;
	//instruction_map[0xFC] null
	//instruction_map[0xFD] null
	instruction_map[0xFE] = &CPUCore::cp_n;
	instruction_map[0xFF] = &CPUCore::rst_nn;

	//CB extension tables
	RANGE_FILL(ext_instruction_map, 0x00, 0x08, rlc_r, rlc_hl);
	RANGE_FILL(ext_instruction_map, 0x08, 0x10, rrc_r, rrc_hl);
	RANGE_FILL(ext_instruction_map, 0x10, 0x18, rl_r, rl_hl);
	RANGE_FILL(ext_instruction_map, 0x18, 0x20, rr_r, rr_hl);
	RANGE_FILL(ext_instruction_map, 0x20, 0x28, sla_r, sla_hl);
	RANGE_FILL(ext_instruction_map, 0x28, 0x30, sra_r, sra_hl);
	RANGE_FILL(ext_instruction_map, 0x30, 0x38, swap_r, swap_hl);
	RANGE_FILL(ext_instruction_map, 0x38, 0x40, srl_r, srl_hl);
	
	for (int i = 0x40; i < 0x80; ++i)
		ext_instruction_map[i] = &CPUCore::bit_r_x;

	for (int i = 0x46; i < 0x80; i += 8)
		ext_instruction_map[i] = &CPUCore::bit_hl_x;

	for (int i = 0x80; i < 0xC0; ++i)
		ext_instruction_map[i] = &CPUCore::res_r_x;

	for (int i = 0x86; i < 0xC0; i += 8)
		ext_instruction_map[i] = &CPUCore::res_hl_x;

	for (int i = 0xC0; i < 0x100; ++i)
		ext_instruction_map[i] = &CPUCore::set_r_x;

	for (int i = 0xC6; i < 0x100; i += 8)
		ext_instruction_map[i] = &CPUCore::set_hl_x;
}

#undef RANGE_FILL

void CPU::attach_memory(MMU* memory_controller)
{
	mmu = memory_controller;
}

void CPU::reset()
{
	is_halted = false;
	interrupts = false;

	reg_8[A] = 0x01;
	reg_8[F] = 0xB0;
	reg_8[B] = 0x00;
	reg_8[C] = 0x13;
	reg_8[D] = 0x00;
	reg_8[E] = 0xD8;
	reg_8[H] = 0x01;
	reg_8[L] = 0x4D;

	reg_16[SP] = 0xFFFE;
	pc = 0x100;
}

u32 CPU::step()
{
	if (is_halted)
		return nop(0); 

	u32 cycles_passed = 0;
	u8 opcode = fetch8();
	
	if (opcode != 0xCB)
		cycles_passed = (this->*instruction_map[opcode])(opcode);

	else
	{
		u8 ext_opcode = fetch8();
		cycles_passed = (this->*ext_instruction_map[ext_opcode])(ext_opcode);
	}

	return cycles_passed;
}

u32 CPU::handle_interrupt(INTERRUPTS code)
{
	interrupts = false; //disable all interrupts

	push(pc);
	pc = 0x40 + code * 8; //jump to interrupt handler

	return 20;
}

bool CPU::is_interrupt_enabled() const
{
	return interrupts;
}

void CPU::unhalt()
{
	is_halted = false;
}


