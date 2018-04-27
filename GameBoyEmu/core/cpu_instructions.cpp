////#include "stdafx.h"
#include "cpu.h"

static constexpr u8 reg_map[] = { B, C, D, E, H, L, 0xFF, A }; //instead of 0xFF, add 1 reg to reg_8 named as REG_GUARD

//CPU Interface
//--------------------------------------------------------------------------------------

#define RANGE_FILL(tab, beg, end, f1, f2) do{\
											for(int i = beg; i < end; ++i)\
												{tab[i] = &CPU::f1;}\
												tab[beg + 6] = &CPU::f2;\
											} while(0)

void CPU::fill_instruction_maps()
{
	for (auto& i : instr_map)
		i = &CPU::illegal_op;

	for (auto& i : ext_instr_map)
		i = &CPU::illegal_op;

	instr_map[0x00] = &CPU::nop;
	instr_map[0x01] = &CPU::ld_rr_nn;
	instr_map[0x02] = &CPU::ld_rr_a;
	instr_map[0x03] = &CPU::inc_rr;
	instr_map[0x04] = &CPU::inc_r;
	instr_map[0x05] = &CPU::dec_r;
	instr_map[0x06] = &CPU::ld_r_n;
	instr_map[0x07] = &CPU::rlca;
	instr_map[0x08] = &CPU::ld_nn_sp;
	instr_map[0x09] = &CPU::add_hl_rr;
	instr_map[0x0A] = &CPU::ld_a_adr;
	instr_map[0x0B] = &CPU::dec_rr;
	instr_map[0x0C] = &CPU::inc_r;
	instr_map[0x0D] = &CPU::dec_r;
	instr_map[0x0E] = &CPU::ld_r_n;
	instr_map[0x0F] = &CPU::rrca;

	instr_map[0x10] = &CPU::stop;
	instr_map[0x11] = &CPU::ld_rr_nn;
	instr_map[0x12] = &CPU::ld_rr_a;
	instr_map[0x13] = &CPU::inc_rr;
	instr_map[0x14] = &CPU::inc_r;
	instr_map[0x15] = &CPU::dec_r;
	instr_map[0x16] = &CPU::ld_r_n;
	instr_map[0x17] = &CPU::rla;
	instr_map[0x18] = &CPU::jr_n;
	instr_map[0x19] = &CPU::add_hl_rr;
	instr_map[0x1A] = &CPU::ld_a_adr;
	instr_map[0x1B] = &CPU::dec_rr;
	instr_map[0x1C] = &CPU::inc_r;
	instr_map[0x1D] = &CPU::dec_r;
	instr_map[0x1E] = &CPU::ld_r_n;
	instr_map[0x1F] = &CPU::rra;

	instr_map[0x20] = &CPU::jr_cond_r;
	instr_map[0x21] = &CPU::ld_rr_nn;
	instr_map[0x22] = &CPU::ldi_hl_a;
	instr_map[0x23] = &CPU::inc_rr;
	instr_map[0x24] = &CPU::inc_r;
	instr_map[0x25] = &CPU::dec_r;
	instr_map[0x26] = &CPU::ld_r_n;
	instr_map[0x27] = &CPU::daa;
	instr_map[0x28] = &CPU::jr_cond_r;
	instr_map[0x29] = &CPU::add_hl_rr;
	instr_map[0x2A] = &CPU::ldi_a_hl;
	instr_map[0x2B] = &CPU::dec_rr;
	instr_map[0x2C] = &CPU::inc_r;
	instr_map[0x2D] = &CPU::dec_r;
	instr_map[0x2E] = &CPU::ld_r_n;
	instr_map[0x2F] = &CPU::cpl;

	instr_map[0x30] = &CPU::jr_cond_r;
	instr_map[0x31] = &CPU::ld_rr_nn;
	instr_map[0x32] = &CPU::ldd_hl_a;
	instr_map[0x33] = &CPU::inc_rr;
	instr_map[0x34] = &CPU::inc_hl;
	instr_map[0x35] = &CPU::dec_hl;
	instr_map[0x36] = &CPU::ld_hl_n;
	instr_map[0x37] = &CPU::scf;
	instr_map[0x38] = &CPU::jr_cond_r;
	instr_map[0x39] = &CPU::add_hl_rr;
	instr_map[0x3A] = &CPU::ldd_a_hl;
	instr_map[0x3B] = &CPU::dec_rr;
	instr_map[0x3C] = &CPU::inc_r;
	instr_map[0x3D] = &CPU::dec_r;
	instr_map[0x3E] = &CPU::ld_r_n;
	instr_map[0x3F] = &CPU::ccf;

	for (int i = 0x40; i < 0x70; ++i)
		instr_map[i] = &CPU::ld_r_r;

	for (int i = 0x46; i < 0x70; i += 8)
		instr_map[i] = &CPU::ld_r_hl;

	RANGE_FILL(instr_map, 0x70, 0x78, ld_hl_r, halt);
	RANGE_FILL(instr_map, 0x78, 0x80, ld_r_r, ld_r_hl);
	RANGE_FILL(instr_map, 0x80, 0x88, add_a_r, add_a_hl);
	RANGE_FILL(instr_map, 0x88, 0x90, adc_a_r, adc_a_hl);
	RANGE_FILL(instr_map, 0x90, 0x98, sub_a_r, sub_a_hl);
	RANGE_FILL(instr_map, 0x98, 0xA0, sbc_a_r, sbc_a_hl);
	RANGE_FILL(instr_map, 0xA0, 0xA8, and_a_r, and_a_hl);
	RANGE_FILL(instr_map, 0xA8, 0xB0, xor_a_r, xor_a_hl);
	RANGE_FILL(instr_map, 0xB0, 0xB8, or_a_r, or_a_hl);
	RANGE_FILL(instr_map, 0xB8, 0xC0, cp_a_r, cp_a_hl);

	// C0
	instr_map[0xC0] = &CPU::ret_cond;
	instr_map[0xC1] = &CPU::pop_rr;
	instr_map[0xC2] = &CPU::jp_cond_nn;
	instr_map[0xC3] = &CPU::jp_nn;
	instr_map[0xC4] = &CPU::call_cond_nn;
	instr_map[0xC5] = &CPU::push_rr;
	instr_map[0xC6] = &CPU::add_a_n;
	instr_map[0xC7] = &CPU::rst_nn;
	instr_map[0xC8] = &CPU::ret_cond;
	instr_map[0xC9] = &CPU::ret;
	instr_map[0xCA] = &CPU::jp_cond_nn;
	//instr_map[0xCB] extension
	instr_map[0xCC] = &CPU::call_cond_nn;
	instr_map[0xCD] = &CPU::call_nn;
	instr_map[0xCE] = &CPU::adc_a_n;
	instr_map[0xCF] = &CPU::rst_nn;

	// D0
	instr_map[0xD0] = &CPU::ret_cond;
	instr_map[0xD1] = &CPU::pop_rr;
	instr_map[0xD2] = &CPU::jp_cond_nn;
	//instr_map[0xD3] null
	instr_map[0xD4] = &CPU::call_cond_nn;
	instr_map[0xD5] = &CPU::push_rr;
	instr_map[0xD6] = &CPU::sub_a_n;
	instr_map[0xD7] = &CPU::rst_nn;
	instr_map[0xD8] = &CPU::ret_cond;
	instr_map[0xD9] = &CPU::reti;
	instr_map[0xDA] = &CPU::jp_cond_nn;
	//instr_map[0xDB] null
	instr_map[0xDC] = &CPU::call_cond_nn;
	//instr_map[0xDD] null
	instr_map[0xDE] = &CPU::sbc_a_n;
	instr_map[0xDF] = &CPU::rst_nn;

	// E0
	instr_map[0xE0] = &CPU::ldh_n_a;
	instr_map[0xE1] = &CPU::pop_rr;
	instr_map[0xE2] = &CPU::ldh_c_a;
	//instr_map[0xE3] null
	//instr_map[0xE4] null
	instr_map[0xE5] = &CPU::push_rr;
	instr_map[0xE6] = &CPU::and_n;
	instr_map[0xE7] = &CPU::rst_nn;
	instr_map[0xE8] = &CPU::add_sp_n;
	instr_map[0xE9] = &CPU::jp_hl;
	instr_map[0xEA] = &CPU::ld_nn_a;
	//instr_map[0xEB] null
	//instr_map[0xEC] null
	//instr_map[0xED] null
	instr_map[0xEE] = &CPU::xor_n;
	instr_map[0xEF] = &CPU::rst_nn;

	// F0
	instr_map[0xF0] = &CPU::ldh_a_n;
	instr_map[0xF1] = &CPU::pop_af;
	instr_map[0xF2] = &CPU::ldh_a_c;
	instr_map[0xF3] = &CPU::di;
	//instr_map[0xF4] null
	instr_map[0xF5] = &CPU::push_af;
	instr_map[0xF6] = &CPU::or_n;
	instr_map[0xF7] = &CPU::rst_nn;
	instr_map[0xF8] = &CPU::ld_hl_sp_n;
	instr_map[0xF9] = &CPU::ld_sp_hl;
	instr_map[0xFA] = &CPU::ld_a_nn;
	instr_map[0xFB] = &CPU::ei;
	//instr_map[0xFC] null
	//instr_map[0xFD] null
	instr_map[0xFE] = &CPU::cp_n;
	instr_map[0xFF] = &CPU::rst_nn;

	//CB extension tables
	RANGE_FILL(ext_instr_map, 0x00, 0x08, rlc_r, rlc_hl);
	RANGE_FILL(ext_instr_map, 0x08, 0x10, rrc_r, rrc_hl);
	RANGE_FILL(ext_instr_map, 0x10, 0x18, rl_r, rl_hl);
	RANGE_FILL(ext_instr_map, 0x18, 0x20, rr_r, rr_hl);
	RANGE_FILL(ext_instr_map, 0x20, 0x28, sla_r, sla_hl);
	RANGE_FILL(ext_instr_map, 0x28, 0x30, sra_r, sra_hl);
	RANGE_FILL(ext_instr_map, 0x30, 0x38, swap_r, swap_hl);
	RANGE_FILL(ext_instr_map, 0x38, 0x40, srl_r, srl_hl);
	
	for (int i = 0x40; i < 0x80; ++i)
		ext_instr_map[i] = &CPU::bit_r_x;

	for (int i = 0x46; i < 0x80; i += 8)
		ext_instr_map[i] = &CPU::bit_hl_x;

	for (int i = 0x80; i < 0xC0; ++i)
		ext_instr_map[i] = &CPU::res_r_x;

	for (int i = 0x86; i < 0xC0; i += 8)
		ext_instr_map[i] = &CPU::res_hl_x;

	for (int i = 0xC0; i < 0x100; ++i)
		ext_instr_map[i] = &CPU::set_r_x;

	for (int i = 0xC6; i < 0x100; i += 8)
		ext_instr_map[i] = &CPU::set_hl_x;
}

#undef RANGE_FILL

//internal helpers
//--------------------------------------------------------------------------------------
void CPU::push(u16 value, u32 cach_up_cycles)
{
	mmu.write_byte(--reg_16[SP], (value >> 8) & 0xFF, cycles_ahead + cach_up_cycles);
	mmu.write_byte(--reg_16[SP], value & 0xFF, cycles_ahead + cach_up_cycles + 4);
}

u16 CPU::pop(u32 cach_up_cycles)
{
	u8 low = mmu.read_byte(reg_16[SP]++, cycles_ahead + cach_up_cycles);
	u8 high = mmu.read_byte(reg_16[SP]++, cycles_ahead + cach_up_cycles + 4);

	return (high << 8) | low;
}

u8 CPU::read_byte(u16 adress, u32 cach_up_cycles)
{
	return mmu.read_byte(adress, cycles_ahead + cach_up_cycles);
}

void CPU::write_byte(u16 adress, u8 value, u32 cach_up_cycles)
{
	mmu.write_byte(adress, value, cycles_ahead + cach_up_cycles);
}

void CPU::write_word(u16 adress, u16 value, u32 cach_up_cycles)
{
	mmu.write_byte(adress, value & 0x00FF, cycles_ahead + cach_up_cycles);
	mmu.write_byte(adress + 1, (value >> 8) & 0x00FF, cycles_ahead + cach_up_cycles + 4);
}

u16 CPU::read_word(u16 adress, u32 cach_up_cycles)
{
	auto low = mmu.read_byte(adress, cycles_ahead + cach_up_cycles);
	auto high = mmu.read_byte(adress + 1, cycles_ahead + cach_up_cycles + 4);

	return (high << 8) | low;
}

u8 CPU::fetch8(u32 cach_up_cycles)
{
	return mmu.read_byte(pc++, cycles_ahead + cach_up_cycles);
}

u16 CPU::fetch16(u32 cach_up_cycles)
{
	auto ret = read_word(pc, cycles_ahead + cach_up_cycles);
	pc += 2;

	return ret;
}

//CPU opcodes
//---------------------------------------------------------------------------------------

u32 CPU::illegal_op(u8 opcode)
{
	UNUSED(opcode);
	assert(0 && "Accessed unimplemented instruction!");
	return 0;
}

u32 CPU::nop(u8 unused)
{
	UNUSED(unused);
	return 4;
}

u32 CPU::ld_rr_nn(u8 opcode)
{
	reg_16[(opcode >> 4) & 3] = fetch16(4);
	return 12;
}

u32 CPU::ld_rr_a(u8 opcode)
{
	write_byte(reg_16[(opcode >> 4) & 3], reg_8[A], 4);
	return 8;
}

u32 CPU::inc_rr(u8 opcode)
{
	++reg_16[(opcode >> 4) & 3];
	return 8; 
}

u32 CPU::inc_r(u8 opcode)
{
	u8 after = reg_8[reg_map[(opcode >> 3) & 7]] + 1;

	reg_8[F] = change_bit(reg_8[F], after == 0, F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (after & 0xF) == 0x0, F_H);

	reg_8[reg_map[(opcode >> 3) & 7]] = after;

	return 4;
}

u32 CPU::dec_r(u8 opcode)
{
	u8 after = reg_8[reg_map[(opcode >> 3) & 7]] - 1;

	reg_8[F] = change_bit(reg_8[F], after == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (after & 0xF) == 0xF, F_H);

	reg_8[reg_map[(opcode >> 3) & 7]] = after;

	return 4;
}

u32 CPU::ld_r_n(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = fetch8(4);
	return 8;
}

u32 CPU::rlca(u8 opcode)
{
	UNUSED(opcode);
	reg_8[F] = change_bit(0, check_bit(reg_8[A], 7), F_C);
	reg_8[A] = rol(reg_8[A], 1);

	return 4;
}

u32 CPU::ld_nn_sp(u8 opcode)
{
	UNUSED(opcode);
	u16 adr = fetch16(4);
	write_word(adr, reg_16[SP], 12);
	return 20;
}

u32 CPU::add_hl_rr(u8 opcode)
{
	u32 result = reg_16[HL] + reg_16[(opcode >> 4) & 3];

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (reg_16[HL] & 0x0FFF) + (reg_16[(opcode >> 4) & 3] & 0x0FFF) > 0x0FFF, F_H);
	reg_8[F] = change_bit(reg_8[F], result > 0xFFFF, F_C);

	reg_16[HL] = result;
	return 8;
}

u32 CPU::ld_a_adr(u8 opcode)
{
	reg_8[A] = read_byte(reg_16[(opcode >> 4) & 3], 4);
	return 8;
}

u32 CPU::dec_rr(u8 opcode)
{
	--reg_16[(opcode >> 4) & 3];
	return 8;
}

u32 CPU::rrca(u8 opcode)
{
	UNUSED(opcode);
	reg_8[F] = change_bit(0, check_bit(reg_8[A], 0), F_C);
	reg_8[A] = ror(reg_8[A], 1);
	return 4;
}

u32 CPU::stop(u8 opcode)
{
	if (cgb_mode && check_bit(mmu.read_byte(0xFF4D, 0), 0))
	{
		mmu.write_byte(0xFF4D, 0xFF, 0xFFFFFFFF); //0xFF & 0xFFFFFFFF are secret key for speed switch
		return 4;
	}

	else
		return halt(opcode);
}

u32 CPU::rla(u8 opcode)
{
	UNUSED(opcode);

	auto carry = check_bit(reg_8[A], 7);
	reg_8[A] = change_bit(reg_8[A] << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[F] = change_bit(0, carry, F_C);

	return 4;
}

u32 CPU::jr_n(u8 opcode)
{
	UNUSED(opcode);

	pc += static_cast<i8>(fetch8(4));
	return 12;
}

u32 CPU::rra(u8 opcode)
{
	UNUSED(opcode);

	auto carry = check_bit(reg_8[A], 0);
	reg_8[A] = change_bit(reg_8[A] >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[F] = change_bit(0, carry, F_C);
	return 4;
}

u32 CPU::jr_cond_r(u8 opcode)
{
	bool cond = false;

	switch ((opcode >> 3) & 3)
	{
		case 0:
			cond = !check_bit(reg_8[F], F_Z);
			break;

		case 1:
			cond = check_bit(reg_8[F], F_Z);
			break;

		case 2:
			cond = !check_bit(reg_8[F], F_C);
			break;

		case 3:
			cond = check_bit(reg_8[F], F_C);
			break;
	}

	if (cond)
	{
		pc += static_cast<i8>(fetch8(4));
		return 12;
	}

	else
	{
		pc++;
		return 8;
	}
}

u32 CPU::ldi_hl_a(u8 opcode)
{
	UNUSED(opcode);

	write_byte(reg_16[HL]++, reg_8[A], 4);
	return 8;
}

u32 CPU::daa(u8 opcode)
{
	UNUSED(opcode);

	if (check_bit(reg_8[F], F_N))
	{
		if (check_bit(reg_8[F], F_H))
			reg_8[A] += 0xFA;

		if (check_bit(reg_8[F], F_C))
			reg_8[A] += 0xA0;
	}
	
	else
	{
		u32 tmp = reg_8[A];

		if (check_bit(reg_8[F], F_H) || (tmp & 0xF) > 0x09)
			tmp += 0x6;

		if (check_bit(reg_8[F], F_C) || tmp > 0x9F)
		{
			tmp += 0x60;
			reg_8[F] = set_bit(reg_8[F], F_C);
		}

		else
			reg_8[F] = clear_bit(reg_8[F], F_C);

		reg_8[A] = tmp & 0xFF;
	}

	reg_8[F] = clear_bit(reg_8[F], F_H);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == 0, F_Z);

	return 4;
}

u32 CPU::ldi_a_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] = read_byte(reg_16[HL]++, 4);
	return 8;
}

u32 CPU::cpl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] = ~reg_8[A];
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 4;
}

u32 CPU::ldd_hl_a(u8 opcode)
{
	UNUSED(opcode);

	write_byte(reg_16[HL]--, reg_8[A], 4);
	return 8;
}

u32 CPU::inc_hl(u8 opcode)
{
	UNUSED(opcode);

	u8 val = read_byte(reg_16[HL], 4) + 1;

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], (val & 0xF) == 0, F_H);
	
	write_byte(reg_16[HL], val, 8);
	return 12;
}

u32 CPU::dec_hl(u8 opcode)
{
	UNUSED(opcode);

	u16 val = read_byte(reg_16[HL], 4) - 1;

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], (val & 0xF) == 0xF, F_H);

	write_byte(reg_16[HL], val, 8);

	return 12;
}

u32 CPU::ld_hl_n(u8 opcode)
{
	UNUSED(opcode);

	auto val = fetch8(4);
	write_byte(reg_16[HL], val, 8);
	return 12;
}

u32 CPU::scf(u8 opcode)
{
	UNUSED(opcode);

	reg_8[F] = set_bit(reg_8[F], F_C);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = clear_bit(reg_8[F], F_H);

	return 4;
}

u32 CPU::ldd_a_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] = read_byte(reg_16[HL]--, 4);
	return 8;
}

u32 CPU::ccf(u8 opcode)
{
	UNUSED(opcode);

	reg_8[F] = toggle_bit(reg_8[F], F_C);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = clear_bit(reg_8[F], F_H);

	return 4;
}

u32 CPU::ld_r_r(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = reg_8[reg_map[opcode & 7]];
	return 4;
}

u32 CPU::ld_r_hl(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = read_byte(reg_16[HL], 4);
	return 8;
}

u32 CPU::ld_hl_r(u8 opcode)
{
	write_byte(reg_16[HL], reg_8[reg_map[opcode & 7]], 4);
	return 8;
}

u32 CPU::halt(u8 opcode)
{
	UNUSED(opcode);

	is_halted = true;
	return 4;
}

u32 CPU::add_a_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	u32 result = reg_8[A] + reg;
	
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ reg ^ result) & 0x10, F_H);

	reg_8[A] = result;
	return 4;
}

u32 CPU::add_a_hl(u8 opcode)
{
	UNUSED(opcode);

	u8 val = read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] + val; 
	
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPU::adc_a_r(u8 opcode)
{
	auto reg = reg_8[reg_map[opcode & 7]];
	u32 result = reg_8[A] + reg + check_bit(reg_8[F], F_C);

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ reg ^ result) & 0x10, F_H);

	reg_8[A] = result;
	return 4;
}

u32 CPU::adc_a_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] + val + check_bit(reg_8[F], F_C);

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPU::sub_a_r(u8 opcode)
{
	u8 val = reg_8[reg_map[opcode & 7]];
	u32 result = reg_8[A] - val; 
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	reg_8[A] = result;
	return 4;
}

u32 CPU::sub_a_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] - val;

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPU::sbc_a_r(u8 opcode) 
{
	auto reg = reg_8[reg_map[opcode & 7]];
	i32 result = reg_8[A] - reg - check_bit(reg_8[F], F_C);
	i32 half_result = (reg_8[A] & 0xF) - (reg & 0xF) - check_bit(reg_8[F], F_C);
	reg_8[A] = result;

	reg_8[F] = change_bit(reg_8[F], reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], result < 0, F_C);
	reg_8[F] = change_bit(reg_8[F], half_result < 0 , F_H); 

	return 4;
}

u32 CPU::sbc_a_hl(u8 opcode) 
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 4);
	i32 result = reg_8[A] - val - check_bit(reg_8[F], F_C);
	i32 half_result = (reg_8[A] & 0xF) - (val & 0xF) - check_bit(reg_8[F], F_C);
	reg_8[A] = static_cast<u8>(result);

	reg_8[F] = change_bit(reg_8[F], reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], result < 0, F_C);
	reg_8[F] = change_bit(reg_8[F], half_result < 0, F_H);
	
	return 8;
}

u32 CPU::and_a_r(u8 opcode)
{
	reg_8[A] &= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 4;
}

u32 CPU::and_a_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] &= read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPU::xor_a_r(u8 opcode)
{
	reg_8[A] ^= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 4;
}

u32 CPU::xor_a_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] ^= read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 8;
}

u32 CPU::or_a_r(u8 opcode)
{
	reg_8[A] |= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 4;
}

u32 CPU::or_a_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] |= read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 8;
}

u32 CPU::cp_a_r(u8 opcode)
{
	u8 val = reg_8[reg_map[opcode & 7]];

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	return 4;
}

u32 CPU::cp_a_hl(u8 opcode)
{
	UNUSED(opcode);

	u8 val = read_byte(reg_16[HL], 4);

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);
		
	return 8;
}

u32 CPU::ret_cond(u8 opcode)
{
	bool cond = false;

	switch ((opcode >> 3) & 3)
	{
		case 0:
			cond = !check_bit(reg_8[F], F_Z);
			break;

		case 1:
			cond = check_bit(reg_8[F], F_Z);
			break;

		case 2:
			cond = !check_bit(reg_8[F], F_C);
			break;

		case 3:
			cond = check_bit(reg_8[F], F_C);
			break;
	}

	if (cond)
	{
		pc = pop(8);
		return 20;
	}

	else
		return 8;
}

u32 CPU::pop_rr(u8 opcode)
{
	reg_16[(opcode >> 4) & 3] = pop(4); 
	return 12;
}

u32 CPU::jp_cond_nn(u8 opcode)
{
	bool cond = false;

	switch ((opcode >> 3) & 3)
	{
		case 0:
			cond = !check_bit(reg_8[F], F_Z);
			break;

		case 1:
			cond = check_bit(reg_8[F], F_Z);
			break;

		case 2:
			cond = !check_bit(reg_8[F], F_C);
			break;

		case 3:
			cond = check_bit(reg_8[F], F_C);
			break;
	}

	if (cond)
	{
		pc = fetch16(4);
		return 16;
	}

	else
	{
		pc += 2; 
		return 12;
	}
}

u32 CPU::jp_nn(u8 opcode)
{
	UNUSED(opcode);

	pc = fetch16(4);
	return 16;
}

u32 CPU::call_cond_nn(u8 opcode)
{
	bool cond = false;

	switch ((opcode >> 3) & 3)
	{
		case 0:
			cond = !check_bit(reg_8[F], F_Z);
			break;

		case 1:
			cond = check_bit(reg_8[F], F_Z);
			break;

		case 2:
			cond = !check_bit(reg_8[F], F_C);
			break;

		case 3:
			cond = check_bit(reg_8[F], F_C);
			break;
	}

	if (cond)
	{
		u16 new_pc = pc + 2;
		pc = fetch16(4);
		push(new_pc, 16);
		return 24;
	}

	else
	{
		pc += 2; 
		return 12;
	}
}

u32 CPU::push_rr(u8 opcode)
{
	push(reg_16[(opcode >> 4) & 3], 8);
	return 16;
}

u32 CPU::add_a_n(u8 opcode)
{
	UNUSED(opcode);

	u8 val = fetch8(4);
	u32 result = reg_8[A] + val; 

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], ((reg_8[A] & 0x0F) + (val & 0x0F)) > 0xF, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPU::rst_nn(u8 opcode)
{
	u16 adr = (opcode >> 3) & 7;
	push(pc, 8); 
	pc = adr * 0x8; 

	return 16;
}

u32 CPU::ret(u8 opcode)
{
	UNUSED(opcode);

	pc = pop(4);
	return 16;
}

u32 CPU::call_nn(u8 opcode)
{
	UNUSED(opcode);

	u16 new_pc = pc + 2;
	pc = fetch16(4);
	push(new_pc, 16);
	return 24;
}

u32 CPU::adc_a_n(u8 opcode)
{
	UNUSED(opcode);

	u8 val = fetch8(4);
	u32 result = reg_8[A] + val + check_bit(reg_8[F], F_C);

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPU::sub_a_n(u8 opcode) 
{
	UNUSED(opcode);

	u8 val = fetch8(4);
	u32 result = reg_8[A] - val; 
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPU::reti(u8 opcode)
{
	UNUSED(opcode);

	pc = pop(4);
	interrupts = true;
	return 16;
}

u32 CPU::sbc_a_n(u8 opcode) 
{
	UNUSED(opcode);

	u8 val = fetch8(4);
	i32 result = reg_8[A] - val - check_bit(reg_8[F], F_C);
	i32 half_result = (reg_8[A] & 0xF) - (val & 0xF) - check_bit(reg_8[F], F_C);
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z); 
	reg_8[F] = change_bit(reg_8[F], result < 0, F_C);
	reg_8[F] = change_bit(reg_8[F], half_result < 0, F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPU::ldh_n_a(u8 opcode)
{
	UNUSED(opcode);

	auto val = fetch8(4);
	write_byte(0xFF00 + val, reg_8[A], 8);
	return 12;
}

u32 CPU::ldh_c_a(u8 opcode)
{
	UNUSED(opcode);

	write_byte(0xFF00 + reg_8[C], reg_8[A], 4);
	return 8;
}

u32 CPU::and_n(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] &= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPU::add_sp_n(u8 opcode)
{
	UNUSED(opcode);

	i8 val = static_cast<i8>(fetch8(4));
	u16 reg = reg_16[SP];
	reg_16[SP] += val;

	u16 tmp = reg_16[SP] ^ val ^ reg;
	reg_8[F] = change_bit(0, !!(tmp & 0x10), F_H);
	reg_8[F] = change_bit(reg_8[F], !!(tmp & 0x100), F_C);

	return 16;
}

u32 CPU::jp_hl(u8 opcode)
{
	UNUSED(opcode);

	pc = reg_16[HL];
	return 4;
}

u32 CPU::ld_nn_a(u8 opcode)
{
	UNUSED(opcode);

	auto adr = fetch16(4);
	write_byte(adr, reg_8[A], 12);
	return 16;
}

u32 CPU::xor_n(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] ^= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	return 8;
}

u32 CPU::ldh_a_n(u8 opcode)
{
	UNUSED(opcode);

	auto val = fetch8(4);
	reg_8[A] = read_byte(0xFF00 + val, 8);
	return 12;
}

u32 CPU::pop_af(u8 opcode)
{
	UNUSED(opcode);

	reg_16[AF] = pop(4);
	reg_8[F] &= 0xF0;
	return 12;
}

u32 CPU::ldh_a_c(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] = read_byte(0xFF00 + reg_8[C], 4);
	return 8;
}

u32 CPU::di(u8 opcode)
{
	UNUSED(opcode);

	interrupts = false;
	return 4;
}

u32 CPU::push_af(u8 opcode)
{
	UNUSED(opcode);

	push(reg_16[AF], 8);
	return 16;
}

u32 CPU::or_n(u8 opcode)
{
	UNUSED(opcode);

	reg_8[A] |= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	return 8;
}

u32 CPU::ld_hl_sp_n(u8 opcode)
{
	UNUSED(opcode);

	i8 val = static_cast<i8>(fetch8(4));
	u16 result = reg_16[SP] + val; 
	u16 tmp = reg_16[SP] ^ result ^ val;

	reg_8[F] = change_bit(0, !!(tmp & 0x10), F_H);
	reg_8[F] = change_bit(reg_8[F], !!(tmp & 0x100), F_C);

	reg_16[HL] = result;

	return 12;
}

u32 CPU::ld_sp_hl(u8 opcode)
{
	UNUSED(opcode);

	reg_16[SP] = reg_16[HL];
	return 8;
}

u32 CPU::ld_a_nn(u8 opcode)
{
	UNUSED(opcode);

	auto adr = fetch16(4);
	reg_8[A] = read_byte(adr, 12); 
	return 16;
}

u32 CPU::ei(u8 opcode)
{
	UNUSED(opcode);

	delayed_ei = true;
	return 4;
}

u32 CPU::cp_n(u8 opcode)
{
	UNUSED(opcode);

	u8 val = fetch8(4);
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	return 8;
}

//CB opcodes
//-----------------------------------------------------------------------------------------

u32 CPU::rlc_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 7), F_C);

	reg_8[reg_map[opcode & 7]] = rol(reg_8[reg_map[opcode & 7]], 1);

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);
	return 8;
}

u32 CPU::rlc_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 7), F_C);

	val = rol(val, 1);

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPU::rrc_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 0), F_C);

	reg_8[reg_map[opcode & 7]] = ror(reg_8[reg_map[opcode & 7]], 1);

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);
	return 8;
}

u32 CPU::rrc_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 0), F_C);

	val = ror(val, 1);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPU::rl_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	auto carry = check_bit(reg, 7);
	
	reg = change_bit(reg << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[reg_map[opcode & 7]] = reg;

	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);
	return 8;
}

u32 CPU::rl_hl(u8 opcode)
{
	UNUSED(opcode);

	u8 val = read_byte(reg_16[HL], 8);
	auto carry = check_bit(val, 7);

	val = change_bit(val << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPU::rr_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	auto carry = check_bit(reg, 0);

	reg = change_bit(reg >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[reg_map[opcode & 7]] = reg;

	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);

	return 8;
}

u32 CPU::rr_hl(u8 opcode)
{
	UNUSED(opcode);

	u8 val = read_byte(reg_16[HL], 8);
	auto carry = check_bit(val, 0);

	val = change_bit(val >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	
	write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPU::sla_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 7), F_C);

	reg_8[reg_map[opcode & 7]] <<= 1;

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);

	return 8;
}

u32 CPU::sla_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 7), F_C);

	val <<= 1;

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPU::sra_r(u8 opcode)
{
	auto r = reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, check_bit(r, 0), F_C);

	r = (r >> 1) | (r & 0x80);

	reg_8[F] = change_bit(reg_8[F], r == 0, F_Z);
	reg_8[reg_map[opcode & 7]] = r;

	return 8;
}

u32 CPU::sra_hl(u8 opcode)
{
	UNUSED(opcode);

	auto r = read_byte(reg_16[HL], 8);

	reg_8[F] = change_bit(0, check_bit(r, 0), F_C);

	r = (r >> 1) | (r & 0x80);

	reg_8[F] = change_bit(reg_8[F], r == 0, F_Z);
	write_byte(reg_16[HL], r, 12);

	return 16;
}

u32 CPU::swap_r(u8 opcode)
{
	reg_8[reg_map[opcode & 7]] = rol(reg_8[reg_map[opcode & 7]], 4);
	reg_8[F] = change_bit(0, reg_8[reg_map[opcode & 7]] == 0, F_Z);

	return 8;
}

u32 CPU::swap_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 8);
	val = rol(val, 4);

	reg_8[F] = change_bit(0, val == 0, F_Z);
	write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPU::srl_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, check_bit(reg, 0), F_C);

	reg >>= 1;

	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);
	reg_8[reg_map[opcode & 7]] = reg;
	return 8;
}

u32 CPU::srl_hl(u8 opcode)
{
	UNUSED(opcode);

	auto val = read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 0), F_C);

	val >>= 1;

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPU::bit_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;

	reg_8[F] = change_bit(reg_8[F], !check_bit(reg_8[reg_map[opcode & 7]], bit), F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPU::bit_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = read_byte(reg_16[HL], 8);

	reg_8[F] = change_bit(reg_8[F], !check_bit(val, bit), F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 12;
}

u32 CPU::res_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	reg_8[reg_map[opcode & 7]] = clear_bit(reg_8[reg_map[opcode & 7]], bit);

	return 8;
}

u32 CPU::res_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = read_byte(reg_16[HL], 8);

	val = clear_bit(val, bit);

	write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPU::set_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	reg_8[reg_map[opcode & 7]] = set_bit(reg_8[reg_map[opcode & 7]], bit);
	return 8;
}

u32 CPU::set_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = read_byte(reg_16[HL], 8);

	val = set_bit(val, bit);

	write_byte(reg_16[HL], val, 12);
	return 16;
}
