#pragma once
#include "stdafx.h"

template<class T>
class CPU_Debugger
{
	private:
		T cpu;
		
		std::vector<u16> break_points;
		bool next_instruction = false;

		bool is_breakpoint();
		void enter_trap();

		const char* dispatch_opcode(u8 opcode, u8 byte_1, u8 byte_2);

	public:
		T& get_cpu();

		void insert_breakpoint(u16 adress);
		void remove_breakpoint(u16 adress);

		void change_register_value(u32 reg, u16 new_val);
		void dump_registers();
		void dump_memory_region(u16 start, u16 end);

		u32 step();
		u32 handle_interrupt(INTERRUPTS code); 
		bool is_interrupt_enabled() const;
		void fill_tabs();
		void attach_memory(MMU* memory_controller);
		void reset();
		void unhalt();
};

template<class T>
inline bool CPU_Debugger<T>::is_breakpoint()
{
	return !break_points.empty() && std::binary_search(break_points.begin(), break_points.end(), cpu.pc);
}

template<class T>
inline void CPU_Debugger<T>::enter_trap()
{
	u8 opcode = cpu.mmu->read_byte(cpu.pc), 
	   b1 = cpu.mmu->read_byte(cpu.pc + 1),
	   b2 = cpu.mmu->read_byte(cpu.pc + 2);

	printf("\nBREAK POINT!\n");
	printf("0x%04x: %s\n", cpu.pc, dispatch_opcode(opcode, b1, b2));
	printf("continue - y, dump registers - d, dump memory - m\n");
	printf("new breakpoint - i, remove breakpoint - r, next instruction - n\n");
	printf("change pc - p\n");// , change reg value - q\n");

	char choice = 0;
	next_instruction = false;

	while (choice != 'y')
	{
		u32 in_row = 0;
		u32 begin = 0, end = 0;
		u32 num = 0;

		scanf("%c", &choice);

		switch (choice)
		{
			case 'n':
				next_instruction = true;
				printf("\n\n");
				return;

			case 'd':
				dump_registers();
				break;

			case 'i':
				printf("\nBreak point adress (hex, 16-bit): ");
				
				if (scanf("%x", &num) && num < 0x10000)
				{
					insert_breakpoint(num & 0xFFFF);
					printf("Break point added!\n");
				}

				else
					printf("Wrong breakpoint adress!\n");

				break;

			case 'r':
				printf("\ncurrent breakpoints:\n");

				for (auto i : break_points)
				{
					printf("0x%04x ", i);

					if (in_row == 4)
						printf("\n");

					in_row = (in_row + 1) % 4;
				}

				printf("\nadress: ");
				
				if (scanf("%x", &in_row))
					remove_breakpoint(in_row);

				else
					printf("wrong adress!\n");

				break;

			case 'm':			
				printf("\nstart adress (hex, 16-bit): ");
				scanf("%x", &begin);
				printf("end adress (hex, 16-bit): ");
				scanf("%x", &end);

				if (begin < 0x10000 && end < 0x10000)
					dump_memory_region(begin, end);

				else
					printf("Adress greater than 0xFFFF!\n");

				break;

			case 'p':
			{
				u32 new_pc = 0;
				printf("\nnew program counter (16bit): ");
				scanf("%x", &new_pc);

				if (new_pc <= 0xFFFF)
					cpu.pc = new_pc;

				else
					printf("Adress greater than 0xFFFF!\n");

				break;
			}

			/*case 'q':
			{
				printf("\nSelect register:\n");
				printf("B - 0,C - 1,D - 2,E - 3,H - 4,L - 5,A - 6,F - 7\n");
				printf("BC,DE,HL,SP,AF\n");

				change_register_value();
				break;
			}*/
		}

	}
}

template<class T>
inline T& CPU_Debugger<T>::get_cpu()
{
	return cpu;
}

template<class T>
inline void CPU_Debugger<T>::insert_breakpoint(u16 adress)
{
	break_points.push_back(adress);
	std::sort(break_points.begin(), break_points.end());
}

template<class T>
inline void CPU_Debugger<T>::remove_breakpoint(u16 adress)
{
	auto it = std::lower_bound(break_points.begin(), break_points.end(), adress);

	if (it != break_points.end())
		break_points.erase(it);
}

template<class T>
inline void CPU_Debugger<T>::change_register_value(u32 reg, u16 new_val)
{
	if (new_val < 0x08)
		cpu.reg_8[reg] = new_val & 0xFF;

	else if (new_val >= 0x10 && new_val <= 0x13)
		cpu.reg_16[reg - 0x10] = new_val;
}

template<class T>
inline void CPU_Debugger<T>::dump_registers()
{
	static const char* regs_16_names[] = { "BC", "DE", "HL", "SP", "AF" };
	int j = 0;

	for (auto i : cpu.reg_16)
		printf("\n%s: 0x%04x", regs_16_names[j++], i);

	printf("\n\nA: 0x%02x  F: 0x%02x\n", cpu.reg_8[A], cpu.reg_8[F]);
	printf("B: 0x%02x  C: 0x%02x\n", cpu.reg_8[B], cpu.reg_8[C]);
	printf("D: 0x%02x  E: 0x%02x\n", cpu.reg_8[D], cpu.reg_8[E]);
	printf("H: 0x%02x  L: 0x%02x\n", cpu.reg_8[H], cpu.reg_8[L]);
	printf("\nFlags (F register): Z:%d C:%d H:%d N:%d\n", check_bit(cpu.reg_8[F], F_Z), check_bit(cpu.reg_8[F], F_C), check_bit(cpu.reg_8[F], F_H), check_bit(cpu.reg_8[F], F_N));
	printf("Interrupts (IME): %s\n", cpu.interrupts ? "enabled" : "disabled");
}

template<class T>
inline void CPU_Debugger<T>::dump_memory_region(u16 start, u16 end)
{
	u32 in_row = 15;

	if (end < start)
		std::swap(start, end);

	printf("\t");

	for (u32 i = 0; i < 16; ++i)
		printf(" %01x ", (start + i) % 16);

	for (u32 i = start; i <= end; ++i)
	{
		if (in_row == 15)
			printf("\n0x%04x: ", i);

		printf("%02x ", cpu.mmu->read_byte(i));

		in_row = (in_row + 1) % 16;
	}

	printf("\n");
}

template<class T>
inline u32 CPU_Debugger<T>::step()
{
	if (next_instruction || is_breakpoint())
		enter_trap();

	return cpu.step();
}

template<class T>
inline u32 CPU_Debugger<T>::handle_interrupt(INTERRUPTS code)
{
	return cpu.handle_interrupt(code);
}

template<class T>
inline bool CPU_Debugger<T>::is_interrupt_enabled() const
{
	return cpu.is_interrupt_enabled();
}

template<class T>
inline void CPU_Debugger<T>::fill_tabs()
{
	cpu.fill_tabs();
}

template<class T>
inline void CPU_Debugger<T>::attach_memory(MMU * memory_controller)
{
	cpu.attach_memory(memory_controller);
}

template<class T>
inline void CPU_Debugger<T>::reset()
{
	cpu.reset();
}

template<class T>
inline void CPU_Debugger<T>::unhalt()
{
	cpu.unhalt();
}

template<class T>
inline const char* CPU_Debugger<T>::dispatch_opcode(u8 opcode, u8 byte_1, u8 byte_2)
{
	static const char* opcodes[] = { "NOP", "LD %s,%s", "INC %s", "DEC %s", "RLCA",
		"ADD %s,%s", "STOP", "RLA", "JR %s", "DAA", "SCF", "HALT", "ADC %s,%s", "SUB %s,%s",
		"SBC %s,%s", "AND %s", "XOR %s", "OR %s", "CP %s", "RET %s", "POP %s", "PUSH %s", 
		"JP %s", "CALL %s", "RST %s", "RETI", "LDH %s,%s", "DI", "EI"};

	static const char* ext_opcodes[] = { "RLC {0}", "RRC {0}", "RL {0}", "RR {0}",
		"SLA {0}", "SRA {0}", "SWAP {0}", "SRL {0}", "BIT {1},{0}", "RES {1},{0}", "SET {1},{0}"};

	static const char* regs[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };

	//static const char* regs_8[] = { "B", "C", "D", "E", "H", "L", "A" };
	//static const char* regs_16[] = { "BC", "DE", "HL", "SP", "AF" };
	//static const char* adr[] = { "(HL)", "(BC)", "(DE)" };
	//static const char* conditions[] = { "NZ", "Z", "NC", "C" };
	//static const char* imm[] = { "%02x","%04x" };

	static const u8 adr_tab[256] = { 0, 1, 1, 2, 2, 3, 1, 4, 1, 5, 1, 3,
	2, 3, 1, 1, 6, 1, 1, 2, 2, 3, 1, 7, 8, 5, 1, 3, 2, 3, 1, 1, 8, 1, 1,
	2, 2, 3, 1, 9, 8, 5, 1, 3, 2, 3, 1, 1, 8, 1, 1, 2, 2, 3, 1, 10, 8,
	5, 1, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 11, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 12, 12, 12, 12, 12, 12, 12, 12, 13,
	13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15,
	15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17,
	17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 20, 22, 22,
	23, 21, 5, 24, 20, 19, 22, 1, 23, 23, 12, 24, 19, 20, 22, 1, 23, 21,
	13, 24, 20, 25, 22, 1, 23, 1, 14, 24, 26, 20, 1, 1, 1, 21, 15, 24, 2,
	22, 1, 1, 1, 1, 16, 24, 26, 20, 1, 27, 1, 21, 17, 24, 1, 1, 1, 28, 1,
	1, 18, 24 };

	if (opcode != 0xCB)
		return opcodes[adr_tab[opcode]];

	else
		return ext_opcodes[byte_1 < 0x40 ? byte_1 / 8 : byte_1 / 0x40 + 7];
}