#pragma once
#include "stdafx.h"
#include "mmu.h"
#include "cpu.h"

class CPUCore : public CPU<CPUCore>
{
	private:
		//internal utility functions
		void push(u16 value, u32 cach_up_cycles);
		u16 pop(u32 cach_up_cycles);

		void write_word(u16 adress, u16 value, u32 cach_up_cycles);
		u16 read_word(u16 adress, u32 cach_up_cycles);

		u8 fetch8(u32 cach_up_cycles);
		u16 fetch16(u32 cach_up_cycles);

	public:
		//CPU interface
		void fill_instruction_maps();
		u8 fetch_op();
		u8 fetch_ext_op();
		u32 interrupt_handler(INTERRUPTS code);

		//CPU instructions
		u32 illegal_op(u8 opcode);

		u32 nop(u8 unused);
		u32 ld_rr_nn(u8 opcode);
		u32 ld_rr_a(u8 opcode);
		u32 inc_rr(u8 opcode);
		u32 inc_r(u8 opcode);
		u32 dec_r(u8 opcode);
		u32 ld_r_n(u8 opcode);
		u32 rlca(u8 opcode);
		u32 ld_nn_sp(u8 opcode);
		u32 add_hl_rr(u8 opcode);
		u32 ld_a_adr(u8 opcode);
		u32 dec_rr(u8 opcode);
		u32 rrca(u8 opcode);
		u32 stop(u8 opcode);
		u32 rla(u8 opcode);
		u32 jr_n(u8 opcode);
		u32 rra(u8 opcode);
		u32 jr_cond_r(u8 opcode);
		u32 ldi_hl_a(u8 opcode);
		u32 daa(u8 opcode);
		u32 ldi_a_hl(u8 opcode);
		u32 cpl(u8 opcode);
		u32 ldd_hl_a(u8 opcode);
		u32 inc_hl(u8 opcode);
		u32 dec_hl(u8 opcode);
		u32 ld_hl_n(u8 opcode);
		u32 scf(u8 opcode);
		u32 ldd_a_hl(u8 opcode);
		u32 ccf(u8 opcode);
		u32 ld_r_r(u8 opcode);
		u32 ld_r_hl(u8 opcode);
		u32 ld_hl_r(u8 opcode);
		u32 halt(u8 opcode);
		u32 add_a_r(u8 opcode);
		u32 add_a_hl(u8 opcode);
		u32 adc_a_r(u8 opcode);
		u32 adc_a_hl(u8 opcode);
		u32 sub_a_r(u8 opcode);
		u32 sub_a_hl(u8 opcode);
		u32 sbc_a_r(u8 opcode);
		u32 sbc_a_hl(u8 opcode);
		u32 and_a_r(u8 opcode);
		u32 and_a_hl(u8 opcode);
		u32 xor_a_r(u8 opcode);
		u32 xor_a_hl(u8 opcode);
		u32 or_a_r(u8 opcode);
		u32 or_a_hl(u8 opcode);
		u32 cp_a_r(u8 opcode);
		u32 cp_a_hl(u8 opcode);
		u32 ret_cond(u8 opcode);
		u32 pop_rr(u8 opcode);
		u32 jp_cond_nn(u8 opcode);
		u32 jp_nn(u8 opcode);
		u32 call_cond_nn(u8 opcode);
		u32 push_rr(u8 opcode);
		u32 add_a_n(u8 opcode);
		u32 rst_nn(u8 opcode);
		u32 ret(u8 opcode);
		u32 call_nn(u8 opcode);
		u32 adc_a_n(u8 opcode);
		u32 sub_a_n(u8 opcode);
		u32 reti(u8 opcode);
		u32 sbc_a_n(u8 opcode);
		u32 ldh_n_a(u8 opcode);
		u32 ldh_c_a(u8 opcode);
		u32 and_n(u8 opcode);
		u32 add_sp_n(u8 opcode);
		u32 jp_hl(u8 opcode);
		u32 ld_nn_a(u8 opcode);
		u32 xor_n(u8 opcode);
		u32 ldh_a_n(u8 opcode);
		u32 pop_af(u8 opcode);
		u32 ldh_a_c(u8 opcode);
		u32 di(u8 opcode);
		u32 push_af(u8 opcode);
		u32 or_n(u8 opcode);
		u32 ld_hl_sp_n(u8 opcode);
		u32 ld_sp_hl(u8 opcode);
		u32 ld_a_nn(u8 opcode);
		u32 ei(u8 opcode);
		u32 cp_n(u8 opcode);

		//extended instructions (CB prefix)
		u32 rlc_r(u8 opcode);
		u32 rlc_hl(u8 opcode);
		u32 rrc_r(u8 opcode);
		u32 rrc_hl(u8 opcode);
		u32 rl_r(u8 opcode);
		u32 rl_hl(u8 opcode);
		u32 rr_r(u8 opcode);
		u32 rr_hl(u8 opcode);
		u32 sla_r(u8 opcode);
		u32 sla_hl(u8 opcode);
		u32 sra_r(u8 opcode);
		u32 sra_hl(u8 opcode);
		u32 swap_r(u8 opcode);
		u32 swap_hl(u8 opcode);
		u32 srl_r(u8 opcode);
		u32 srl_hl(u8 opcode);
		u32 bit_r_x(u8 opcode);
		u32 bit_hl_x(u8 opcode);
		u32 res_r_x(u8 opcode);
		u32 res_hl_x(u8 opcode);
		u32 set_r_x(u8 opcode);
		u32 set_hl_x(u8 opcode);
};