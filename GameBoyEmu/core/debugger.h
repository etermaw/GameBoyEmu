#pragma once


class Debugger
{
	private:
#ifndef ENABLE_AUTO_TESTS 
		std::vector<u16> break_points = { 0x100 };
#else
		std::vector<u16> break_points;
#endif
		std::vector<u16> memory_watches;

		function<u8(u16, u32)> read_byte_callback;
		function<void(u16, u8, u32)> write_byte_callback;
		function<void(std::array<u16, 5>&, bool&)> cpu_state_callback;
		function<void(std::array<u8, 12>&, std::array<u8, 8>&)> gpu_state_callback;

		u16* pc;
		
		u32 vblanks_left = 0;
		u16 step_over_adress = 0;
		u16 change_adress = 0;
		u8 new_val = 0;
		bool next_instruction = false;
		bool memory_changed = false;
		bool step_over = false;

		bool is_breakpoint();
		void enter_trap();

		const char* dispatch_opcode(u8 opcode, u8 byte_1);
		u8 get_opcode_bytes(u8 opcode);

		void insert_breakpoint(u16 adress);
		void remove_breakpoint(u16 adress);

		void insert_watchpoint(u16 adress);
		void remove_watchpoint(u16 adress);

		void dump_registers();
		void dump_memory_region(u16 start, u16 end);
		void dump_gpu_regs();

	public:
		void attach_mmu(function<u8(u16, u32)> read_byte, function<void(u16, u8, u32)> write_byte);
		//void attach_mbc(u32* bank_num);

		std::tuple<u16**, function<void(std::array<u16, 5>&, bool&)>*> get_cpu()
		{
			return std::make_tuple(&pc, &cpu_state_callback);
		}

		void attach_gpu(function<void(std::array<u8, 12>&, std::array<u8, 8>&)> dbg_func) { gpu_state_callback = dbg_func; }

		void check_memory_access(u16 adress, u8 value);
		void step();
		void after_vblank();
};
