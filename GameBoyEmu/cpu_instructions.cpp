#include "stdafx.h"
#include "cpu_instructions.h"

static constexpr u8 reg_map[] = { B, C, D, E, H, L, -1, A }; //instead of -1, add 1 reg to reg_8 named as REG_GUARD

void CPUCore::push(u16 value, u32 cach_up_cycles)
{
	mmu->write_byte(--reg_16[SP], (value >> 8) & 0xFF, cach_up_cycles);
	mmu->write_byte(--reg_16[SP], value & 0xFF, cach_up_cycles + 4);
}

u16 CPUCore::pop(u32 cach_up_cycles)
{
	u8 low = mmu->read_byte(reg_16[SP]++, cach_up_cycles);
	u8 high = mmu->read_byte(reg_16[SP]++, cach_up_cycles + 4);

	return (high << 8) | low;
}

void CPUCore::write_word(u16 adress, u16 value, u32 cach_up_cycles)
{
	mmu->write_byte(adress, value & 0x00FF, cach_up_cycles);
	mmu->write_byte(adress + 1, (value >> 8) & 0x00FF, cach_up_cycles + 4);
}

u16 CPUCore::read_word(u16 adress, u32 cach_up_cycles)
{
	auto low = mmu->read_byte(adress, cach_up_cycles);
	auto high = mmu->read_byte(adress + 1, cach_up_cycles + 4);

	return (high << 8) | low;
}

u8 CPUCore::fetch8(u32 cach_up_cycles)
{
	return mmu->read_byte(pc++, cach_up_cycles);
}

u16 CPUCore::fetch16(u32 cach_up_cycles)
{
	auto ret = read_word(pc, cach_up_cycles);
	pc += 2;

	return ret;
}

u32 CPUCore::illegal_op(u8 opcode)
{
	assert(0 && "Accessed unimplemented instruction!");
	return 0;
}

u32 CPUCore::nop(u8 unused)
{
	return 4;
}

u32 CPUCore::ld_rr_nn(u8 opcode)
{
	reg_16[(opcode >> 4) & 3] = fetch16(4);
	return 12;
}

u32 CPUCore::ld_rr_a(u8 opcode)
{
	mmu->write_byte(reg_16[(opcode >> 4) & 3], reg_8[A], 4);
	return 8;
}

u32 CPUCore::inc_rr(u8 opcode)
{
	++reg_16[(opcode >> 4) & 3];
	return 8; 
}

u32 CPUCore::inc_r(u8 opcode)
{
	u8 after = reg_8[reg_map[(opcode >> 3) & 7]] + 1;

	reg_8[F] = change_bit(reg_8[F], after == 0, F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (after & 0xF) == 0x0, F_H);

	reg_8[reg_map[(opcode >> 3) & 7]] = after;

	return 4;
}

u32 CPUCore::dec_r(u8 opcode)
{
	u8 after = reg_8[reg_map[(opcode >> 3) & 7]] - 1;

	reg_8[F] = change_bit(reg_8[F], after == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (after & 0xF) == 0xF, F_H);

	reg_8[reg_map[(opcode >> 3) & 7]] = after;

	return 4;
}

u32 CPUCore::ld_r_n(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = fetch8(4);
	return 8;
}

u32 CPUCore::rlca(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[A], 7), F_C);
	reg_8[A] = rol(reg_8[A], 1);

	return 4;
}

u32 CPUCore::ld_nn_sp(u8 opcode)
{
	u16 adr = fetch16(4);
	write_word(adr, reg_16[SP], 12); //TODO is it correct value?
	return 20;
}

u32 CPUCore::add_hl_rr(u8 opcode)
{
	u32 result = reg_16[HL] + reg_16[(opcode >> 4) & 3];

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (reg_16[HL] & 0x0FFF) + (reg_16[(opcode >> 4) & 3] & 0x0FFF) > 0x0FFF, F_H);
	reg_8[F] = change_bit(reg_8[F], result > 0xFFFF, F_C);

	reg_16[HL] = result;
	return 8;
}

u32 CPUCore::ld_a_adr(u8 opcode)
{
	reg_8[A] = mmu->read_byte(reg_16[(opcode >> 4) & 3], 4);
	return 8;
}

u32 CPUCore::dec_rr(u8 opcode)
{
	--reg_16[(opcode >> 4) & 3];
	return 8;
}

u32 CPUCore::rrca(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[A], 0), F_C);
	reg_8[A] = ror(reg_8[A], 1);
	return 4;
}

u32 CPUCore::stop(u8 opcode)
{
	return halt(opcode);
}

u32 CPUCore::rla(u8 opcode)
{
	auto carry = check_bit(reg_8[A], 7);
	reg_8[A] = change_bit(reg_8[A] << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[F] = change_bit(0, carry, F_C);

	return 4;
}

u32 CPUCore::jr_n(u8 opcode)
{
	pc += static_cast<i8>(fetch8(4));
	return 12;
}

u32 CPUCore::rra(u8 opcode)
{
	auto carry = check_bit(reg_8[A], 0);
	reg_8[A] = change_bit(reg_8[A] >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[F] = change_bit(0, carry, F_C);
	return 4;
}

u32 CPUCore::jr_cond_r(u8 opcode)
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

u32 CPUCore::ldi_hl_a(u8 opcode)
{
	mmu->write_byte(reg_16[HL]++, reg_8[A], 4);
	return 8;
}

u32 CPUCore::daa(u8 opcode)
{
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

u32 CPUCore::ldi_a_hl(u8 opcode)
{
	reg_8[A] = mmu->read_byte(reg_16[HL]++, 4);
	return 8;
}

u32 CPUCore::cpl(u8 opcode)
{
	reg_8[A] = ~reg_8[A];
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 4;
}

u32 CPUCore::ldd_hl_a(u8 opcode)
{
	mmu->write_byte(reg_16[HL]--, reg_8[A], 4);
	return 8;
}

u32 CPUCore::inc_hl(u8 opcode)
{
	u8 val = mmu->read_byte(reg_16[HL], 4) + 1;

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], (val & 0xF) == 0, F_H);
	
	mmu->write_byte(reg_16[HL], val, 8);
	return 12;
}

u32 CPUCore::dec_hl(u8 opcode)
{
	u16 val = mmu->read_byte(reg_16[HL], 4) - 1;

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], (val & 0xF) == 0xF, F_H);

	mmu->write_byte(reg_16[HL], val, 8);

	return 12;
}

u32 CPUCore::ld_hl_n(u8 opcode)
{
	auto val = fetch8(4);
	mmu->write_byte(reg_16[HL], val, 8);
	return 12;
}

u32 CPUCore::scf(u8 opcode)
{
	reg_8[F] = set_bit(reg_8[F], F_C);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = clear_bit(reg_8[F], F_H);

	return 4;
}

u32 CPUCore::ldd_a_hl(u8 opcode)
{
	reg_8[A] = mmu->read_byte(reg_16[HL]--, 4);
	return 8;
}

u32 CPUCore::ccf(u8 opcode)
{
	reg_8[F] = toggle_bit(reg_8[F], F_C);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = clear_bit(reg_8[F], F_H);

	return 4;
}

u32 CPUCore::ld_r_r(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = reg_8[reg_map[opcode & 7]];
	return 4;
}

u32 CPUCore::ld_r_hl(u8 opcode)
{
	reg_8[reg_map[(opcode >> 3) & 7]] = mmu->read_byte(reg_16[HL], 4);
	return 8;
}

u32 CPUCore::ld_hl_r(u8 opcode)
{
	mmu->write_byte(reg_16[HL], reg_8[reg_map[opcode & 7]], 4);
	return 8;
}

u32 CPUCore::halt(u8 opcode)
{
	is_halted = true;
	return 4;
}

u32 CPUCore::add_a_r(u8 opcode)
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

u32 CPUCore::add_a_hl(u8 opcode)
{
	u8 val = mmu->read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] + val; 
	
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPUCore::adc_a_r(u8 opcode)
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

u32 CPUCore::adc_a_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] + val + check_bit(reg_8[F], F_C);

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPUCore::sub_a_r(u8 opcode)
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

u32 CPUCore::sub_a_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 4);
	u32 result = reg_8[A] - val;

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPUCore::sbc_a_r(u8 opcode) 
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

u32 CPUCore::sbc_a_hl(u8 opcode) 
{
	auto val = mmu->read_byte(reg_16[HL], 4);
	i32 result = reg_8[A] - val - check_bit(reg_8[F], F_C);
	i32 half_result = (reg_8[A] & 0xF) - (val & 0xF) - check_bit(reg_8[F], F_C);
	reg_8[A] = static_cast<u8>(result);

	reg_8[F] = change_bit(reg_8[F], reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], result < 0, F_C);
	reg_8[F] = change_bit(reg_8[F], half_result < 0, F_H);
	
	return 8;
}

u32 CPUCore::and_a_r(u8 opcode)
{
	reg_8[A] &= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 4;
}

u32 CPUCore::and_a_hl(u8 opcode)
{
	reg_8[A] &= mmu->read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPUCore::xor_a_r(u8 opcode)
{
	reg_8[A] ^= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 4;
}

u32 CPUCore::xor_a_hl(u8 opcode)
{
	reg_8[A] ^= mmu->read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 8;
}

u32 CPUCore::or_a_r(u8 opcode)
{
	reg_8[A] |= reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 4;
}

u32 CPUCore::or_a_hl(u8 opcode)
{
	reg_8[A] |= mmu->read_byte(reg_16[HL], 4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);

	return 8;
}

u32 CPUCore::cp_a_r(u8 opcode)
{
	u8 val = reg_8[reg_map[opcode & 7]];

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	return 4;
}

u32 CPUCore::cp_a_hl(u8 opcode)
{
	u8 val = mmu->read_byte(reg_16[HL], 4);

	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);
		
	return 8;
}

u32 CPUCore::ret_cond(u8 opcode)
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

u32 CPUCore::pop_rr(u8 opcode)
{
	reg_16[(opcode >> 4) & 3] = pop(4); 
	return 12;
}

u32 CPUCore::jp_cond_nn(u8 opcode)
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

u32 CPUCore::jp_nn(u8 opcode)
{
	pc = fetch16(4);
	return 16;
}

u32 CPUCore::call_cond_nn(u8 opcode)
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

u32 CPUCore::push_rr(u8 opcode)
{
	push(reg_16[(opcode >> 4) & 3], 4);
	return 16;
}

u32 CPUCore::add_a_n(u8 opcode)
{
	u8 val = fetch8(4);
	u32 result = reg_8[A] + val; 

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], ((reg_8[A] & 0x0F) + (val & 0x0F)) > 0xF, F_H);
	
	reg_8[A] = result;
	return 8;
}

u32 CPUCore::rst_nn(u8 opcode)
{
	u16 adr = (opcode >> 3) & 7;
	push(pc, 4); 
	pc = adr * 0x8; 

	return 16;
}

u32 CPUCore::ret(u8 opcode)
{
	pc = pop(4);
	return 16;
}

u32 CPUCore::call_nn(u8 opcode)
{
	u16 new_pc = pc + 2;
	pc = fetch16(4);
	push(new_pc, 16);
	return 24;
}

u32 CPUCore::adc_a_n(u8 opcode)
{
	u8 val = fetch8(4);
	u32 result = reg_8[A] + val + check_bit(reg_8[F], F_C);

	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] ^ val ^ result) & 0x10, F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPUCore::sub_a_n(u8 opcode) 
{
	u8 val = fetch8(4);
	u32 result = reg_8[A] - val; 
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], (result & 0xFF) == 0, F_Z);
	reg_8[F] = change_bit(reg_8[F], result > 0xFF, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	reg_8[A] = result;
	return 8;
}

u32 CPUCore::reti(u8 opcode)
{
	pc = pop(4);
	interrupts = true;
	return 16;
}

u32 CPUCore::sbc_a_n(u8 opcode) 
{
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

u32 CPUCore::ldh_n_a(u8 opcode)
{
	auto val = fetch8(4);
	mmu->write_byte(0xFF00 + val, reg_8[A], 8);
	return 12;
}

u32 CPUCore::ldh_c_a(u8 opcode)
{
	mmu->write_byte(0xFF00 + reg_8[C], reg_8[A], 4);
	return 8;
}

u32 CPUCore::and_n(u8 opcode)
{
	reg_8[A] &= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPUCore::add_sp_n(u8 opcode)
{
	i8 val = static_cast<i8>(fetch8(4));
	u16 reg = reg_16[SP];
	reg_16[SP] += val;

	u16 tmp = reg_16[SP] ^ val ^ reg;
	reg_8[F] = change_bit(0, !!(tmp & 0x10), F_H);
	reg_8[F] = change_bit(reg_8[F], !!(tmp & 0x100), F_C);

	return 16;
}

u32 CPUCore::jp_hl(u8 opcode)
{
	pc = reg_16[HL];
	return 4;
}

u32 CPUCore::ld_nn_a(u8 opcode)
{
	auto adr = fetch16(4);
	mmu->write_byte(adr, reg_8[A], 12);
	return 16;
}

u32 CPUCore::xor_n(u8 opcode)
{
	reg_8[A] ^= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	return 8;
}

u32 CPUCore::ldh_a_n(u8 opcode)
{
	auto val = fetch8(4);
	reg_8[A] = mmu->read_byte(0xFF00 + val, 8);
	return 12;
}

u32 CPUCore::pop_af(u8 opcode)
{
	reg_16[AF] = pop(4);
	reg_8[F] &= 0xF0;
	return 12;
}

u32 CPUCore::ldh_a_c(u8 opcode)
{
	reg_8[A] = mmu->read_byte(0xFF00 + reg_8[C], 4);
	return 8;
}

u32 CPUCore::di(u8 opcode)
{
	interrupts = false;
	return 4;
}

u32 CPUCore::push_af(u8 opcode) //TODO: somewhere is 4 cycle delay...
{
	push(reg_16[AF], 4);
	return 16;
}

u32 CPUCore::or_n(u8 opcode)
{
	reg_8[A] |= fetch8(4);
	reg_8[F] = change_bit(0, reg_8[A] == 0, F_Z);
	return 8;
}

u32 CPUCore::ld_hl_sp_n(u8 opcode)
{
	i8 val = static_cast<i8>(fetch8(4));
	u16 result = reg_16[SP] + val; 
	u16 tmp = reg_16[SP] ^ result ^ val;

	reg_8[F] = change_bit(0, !!(tmp & 0x10), F_H);
	reg_8[F] = change_bit(reg_8[F], !!(tmp & 0x100), F_C);

	reg_16[HL] = result;

	return 12;
}

u32 CPUCore::ld_sp_hl(u8 opcode)
{
	reg_16[SP] = reg_16[HL];
	return 8;
}

u32 CPUCore::ld_a_nn(u8 opcode)
{
	auto adr = fetch16(4);
	reg_8[A] = mmu->read_byte(adr, 12); 
	return 16;
}

u32 CPUCore::ei(u8 opcode)
{
	interrupts = true;
	return 4;
}

u32 CPUCore::cp_n(u8 opcode)
{
	u8 val = fetch8(4);
	
	reg_8[F] = set_bit(reg_8[F], F_N);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] == val, F_Z);
	reg_8[F] = change_bit(reg_8[F], reg_8[A] < val, F_C);
	reg_8[F] = change_bit(reg_8[F], (reg_8[A] & 0xF) < (val & 0xF), F_H);

	return 8;
}

//CB opcodes
//-----------------------------------------------------------------------------------------

u32 CPUCore::rlc_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 7), F_C);

	reg_8[reg_map[opcode & 7]] = rol(reg_8[reg_map[opcode & 7]], 1);

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);
	return 8;
}

u32 CPUCore::rlc_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 7), F_C);

	val = rol(val, 1);

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	mmu->write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPUCore::rrc_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 0), F_C);

	reg_8[reg_map[opcode & 7]] = ror(reg_8[reg_map[opcode & 7]], 1);

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);
	return 8;
}

u32 CPUCore::rrc_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 0), F_C);

	val = ror(val, 1);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	mmu->write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPUCore::rl_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	auto carry = check_bit(reg, 7);
	
	reg = change_bit(reg << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[reg_map[opcode & 7]] = reg;

	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);
	return 8;
}

u32 CPUCore::rl_hl(u8 opcode)
{
	u8 val = mmu->read_byte(reg_16[HL], 8);
	auto carry = check_bit(val, 7);

	val = change_bit(val << 1, check_bit(reg_8[F], F_C), 0);
	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	mmu->write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPUCore::rr_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	auto carry = check_bit(reg, 0);

	reg = change_bit(reg >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[reg_map[opcode & 7]] = reg;

	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);

	return 8;
}

u32 CPUCore::rr_hl(u8 opcode)
{
	u8 val = mmu->read_byte(reg_16[HL], 8);
	auto carry = check_bit(val, 0);

	val = change_bit(val >> 1, check_bit(reg_8[F], F_C), 7);
	reg_8[F] = change_bit(0, carry, F_C);
	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	
	mmu->write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPUCore::sla_r(u8 opcode)
{
	reg_8[F] = change_bit(0, check_bit(reg_8[reg_map[opcode & 7]], 7), F_C);

	reg_8[reg_map[opcode & 7]] <<= 1;

	reg_8[F] = change_bit(reg_8[F], reg_8[reg_map[opcode & 7]] == 0, F_Z);

	return 8;
}

u32 CPUCore::sla_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 7), F_C);

	val <<= 1;

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);

	mmu->write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPUCore::sra_r(u8 opcode)
{
	auto r = reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, check_bit(r, 0), F_C);

	r = (r >> 1) | (r & 0x80);

	reg_8[F] = change_bit(reg_8[F], r == 0, F_Z);
	reg_8[reg_map[opcode & 7]] = r;

	return 8;
}

u32 CPUCore::sra_hl(u8 opcode)
{
	auto r = mmu->read_byte(reg_16[HL], 8);

	reg_8[F] = change_bit(0, check_bit(r, 0), F_C);

	r = (r >> 1) | (r & 0x80);

	reg_8[F] = change_bit(reg_8[F], r == 0, F_Z);
	mmu->write_byte(reg_16[HL], r, 12);

	return 16;
}

u32 CPUCore::swap_r(u8 opcode)
{
	reg_8[reg_map[opcode & 7]] = rol(reg_8[reg_map[opcode & 7]], 4);
	reg_8[F] = change_bit(0, reg_8[reg_map[opcode & 7]] == 0, F_Z);

	return 8;
}

u32 CPUCore::swap_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 8);
	val = rol(val, 4);

	reg_8[F] = change_bit(0, val == 0, F_Z);
	mmu->write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPUCore::srl_r(u8 opcode)
{
	u8 reg = reg_8[reg_map[opcode & 7]];
	reg_8[F] = change_bit(0, check_bit(reg, 0), F_C);

	reg >>= 1;

	reg_8[F] = change_bit(reg_8[F], reg == 0, F_Z);
	reg_8[reg_map[opcode & 7]] = reg;
	return 8;
}

u32 CPUCore::srl_hl(u8 opcode)
{
	auto val = mmu->read_byte(reg_16[HL], 8);
	reg_8[F] = change_bit(0, check_bit(val, 0), F_C);

	val >>= 1;

	reg_8[F] = change_bit(reg_8[F], val == 0, F_Z);
	mmu->write_byte(reg_16[HL], val, 12);

	return 16;
}

u32 CPUCore::bit_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;

	reg_8[F] = change_bit(reg_8[F], !check_bit(reg_8[reg_map[opcode & 7]], bit), F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 8;
}

u32 CPUCore::bit_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = mmu->read_byte(reg_16[HL], 8);

	reg_8[F] = change_bit(reg_8[F], !check_bit(val, bit), F_Z);
	reg_8[F] = clear_bit(reg_8[F], F_N);
	reg_8[F] = set_bit(reg_8[F], F_H);

	return 12;
}

u32 CPUCore::res_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	reg_8[reg_map[opcode & 7]] = clear_bit(reg_8[reg_map[opcode & 7]], bit);

	return 8;
}

u32 CPUCore::res_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = mmu->read_byte(reg_16[HL], 8);

	val = clear_bit(val, bit);

	mmu->write_byte(reg_16[HL], val, 12);
	return 16;
}

u32 CPUCore::set_r_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	reg_8[reg_map[opcode & 7]] = set_bit(reg_8[reg_map[opcode & 7]], bit);
	return 8;
}

u32 CPUCore::set_hl_x(u8 opcode)
{
	const auto bit = (opcode >> 3) & 7;
	auto val = mmu->read_byte(reg_16[HL], 8);

	val = set_bit(val, bit);

	mmu->write_byte(reg_16[HL], val, 12);
	return 16;
}