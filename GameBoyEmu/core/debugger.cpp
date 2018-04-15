#include "stdafx.h"
#include "debugger.h"

static constexpr char CONTINUE = 'y';
static constexpr char NEXT_INSTR = 'n';
static constexpr char RUN_VB = 'z';
static constexpr char STEP_INSTR = 'l';
static constexpr char INS_BP = 'i';
static constexpr char RM_BP = 'r';
static constexpr char INS_MW = 'q';
static constexpr char RM_MW = 'x';
static constexpr char DMP_REGS = 'd';
static constexpr char DMP_MEM = 'm';
static constexpr char HELP = 'h';
static constexpr char GPU_STATUS = 'g';

static const char help[] = "continue - y, run for x vblanks - z,\n"
						   "dump registers - d, memory dump - m,\n"
						   "new breakpoint - i, remove breakpoint - r,\n"
						   "insert memory watch - q, remove memory watch - x,\n"
						   "next instruction - n, step over instruction - l,\n"
						   "dump GPU regs - g\n";

bool Debugger::is_breakpoint()
{
	return !break_points.empty() && (break_points.find(*pc) != break_points.end());
}

void Debugger::wait_for_user()
{
	u8 opcode = read_byte_callback(*pc, 0);
	char choice = 0;
	next_instruction = false;

	if (step_over_adress == *pc)
		step_over = false;

	while (choice != CONTINUE)
	{
		u32 in_row = 0;
		u32 begin = 0, end = 0;
		u32 num = 0;

		scanf("%c", &choice);

		switch (choice)
		{
			case NEXT_INSTR:
				next_instruction = true;
				return;

			case STEP_INSTR:
				step_over_adress = *pc + (opcode == 0xCB ? 2 : get_opcode_bytes(opcode));
				step_over = true;
				return;

			case DMP_REGS:
				dump_registers();
				break;

			case INS_BP:
				printf("\nBreak point adress (hex, 16-bit): ");

				if (scanf("%x", &num) && num < 0x10000)
				{
					insert_breakpoint(num & 0xFFFF);
					printf("Break point added!\n");
				}

				else
					printf("Wrong breakpoint adress!\n");

				break;

			case RM_BP:
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

			case DMP_MEM:
				printf("\nstart adress (hex, 16-bit): ");
				scanf("%x", &begin);
				printf("end adress (hex, 16-bit): ");
				scanf("%x", &end);

				if (begin < 0x10000 && end < 0x10000)
					dump_memory_region(begin, end);

				else
					printf("Adress greater than 0xFFFF!\n");

				break;

			case INS_MW:
				printf("\nMemory watch adress (hex, 16-bit): ");

				if (scanf("%x", &num) && num < 0x10000)
				{
					insert_watchpoint(num & 0xFFFF);
					printf("Memory watch added!\n");
				}

				else
					printf("Wrong memory watch adress!\n");

				break;

			case RM_MW:
				printf("\ncurrent memory watches:\n");

				for (auto i : memory_watches)
				{
					printf("0x%04x ", i);

					if (in_row == 4)
						printf("\n");

					in_row = (in_row + 1) % 4;
				}

				printf("\nadress: ");

				if (scanf("%x", &in_row))
					remove_watchpoint(in_row);

				else
					printf("wrong adress!\n");

				break;

			case RUN_VB:
				printf("How many vblanks?\n");

				if (scanf("%u", &num) && num < 10000)
				{
					vblanks_left = num;
					printf("\nNext breakpoint after %d vblanks!", vblanks_left);
				}

				else
					printf("\nFailed to add vblank run!");

				break;

			case GPU_STATUS:
				dump_gpu_regs();
				break;

			case HELP:
				printf("%s\n", help);
				break;
		}
	}
}

void Debugger::enter_trap()
{
	u8 opcode = read_byte_callback(*pc, 0),
	   b1 = read_byte_callback(*pc + 1, 0),
	   b2 = read_byte_callback(*pc + 2, 0);

	char buffer[32];
	const char* op = dispatch_opcode(opcode, b1);
	sprintf(buffer, op, get_opcode_bytes(opcode) == 2 ? b1 : (b2 << 8) | b1);

	printf("\nBREAK POINT! (h - help)\n");
	printf("0x%04x: %s\n", *pc, buffer);
}

void Debugger::enter_memory_trap()
{
	printf("\nMEMORY WATCH (h - help):\n");
	printf("CPU wrote 0x%02X to memory location 0x%04X\n", new_val, change_adress);
	printf("Watch triggered on address: 0x%04X\n", current_pc);
}

void Debugger::insert_breakpoint(u16 adress)
{
	break_points.insert(adress);
}

void Debugger::remove_breakpoint(u16 adress)
{
	auto it = break_points.find(adress);

	if (it != break_points.end())
		break_points.erase(it);
}

void Debugger::insert_watchpoint(u16 adress)
{
	memory_watches.insert(adress);
}

void Debugger::remove_watchpoint(u16 adress)
{
	auto it = memory_watches.find(adress);

	if (it != memory_watches.end())
		memory_watches.erase(it);
}

void Debugger::dump_registers()
{
	std::array<u16, 5> regs;
	bool ime;

	cpu_state_callback(regs, ime);

	auto f = regs[4] & 0xFF;

	printf("\nBC: 0x%04x    B: 0x%02x    C: 0x%02x", regs[0], (regs[0] >> 8) & 0xFF, regs[0] & 0xFF);
	printf("\nDE: 0x%04x    D: 0x%02x    E: 0x%02x", regs[1], (regs[1] >> 8) & 0xFF, regs[1] & 0xFF);
	printf("\nHL: 0x%04x    H: 0x%02x    L: 0x%02x", regs[2], (regs[2] >> 8) & 0xFF, regs[2] & 0xFF);
	printf("\nSP: 0x%04x", regs[3]);
	printf("\nAF: 0x%04x    A: 0x%02x    F: 0x%02x", regs[4], (regs[4] >> 8) & 0xFF, regs[4] & 0xFF);

	printf("\n\nFlags (F register): Z:%d N:%d H:%d C:%d\n", check_bit(f, 7), check_bit(f, 6), check_bit(f, 5), check_bit(f, 4));
	printf("Interrupts (IME): %s\n", ime ? "enabled" : "disabled");
}

void Debugger::dump_memory_region(u16 start, u16 end)
{
	if (end < start)
		std::swap(start, end);

	u32 in_row = start % 16;

	printf("\t");

	for (u32 i = 0; i < 16; ++i)
		printf(" %01x ", i);

	if (in_row != 0)
		printf("\n0x%04x: ", start & 0xFFF0);

	for (u32 i = 0; i < in_row; ++i)
		printf("   ");

	for (u32 i = start; i <= end; ++i)
	{
		if (in_row == 0)
			printf("\n0x%04x: ", i & 0xFFF0);

		printf("%02x ", read_byte_callback(i, 0));

		in_row = (in_row + 1) % 16;
	}

	printf("\n");
}

void Debugger::dump_gpu_regs()
{
	static const char* regs[] = { "CTRL","STAT","SY","SX","LY","LYC","DMA","BGP","OBP0","OBP1","WY","WX" };
	static const char* gpu_state[] = {"H-BLANK", "V-BLANK", "OAM", "TRANSFER"};
	std::array<u8, 12> dmg_regs;
	std::array<u8, 8> cgb_regs;

	gpu_state_callback(dmg_regs, cgb_regs);

	for (int i = 0; i < 12; i += 4)
		printf("%s: 0x%02X\t%s: 0x%02X\t%s: 0x%02X\t%s: 0x%02X\n", regs[i], dmg_regs[i], regs[i+1], dmg_regs[i+1], regs[i+2], dmg_regs[i+2], regs[i+3], dmg_regs[i+3]);
	
	printf("\nSTAT:\n[interrupts]\tLYC == LY: %d\t", check_bit(dmg_regs[1],6));
	printf("VB: %d\t", check_bit(dmg_regs[1],4));
	printf("HB: %d\t", check_bit(dmg_regs[1],3));
	printf("OAM: %d\t\n", check_bit(dmg_regs[1],5));
	printf("[read-only]\tLYC == LY cmp bit: %d\t", check_bit(dmg_regs[1],2));
	printf("GPU state: %s\n", gpu_state[dmg_regs[1] & 0x3]);

	printf("\nCTRL:\nLCD: %d ", check_bit(dmg_regs[0], 7));
	printf("BG: %d ", check_bit(dmg_regs[0], 0));
	printf("SPRITES: %d ", check_bit(dmg_regs[0], 1));
	printf("WINDOW: %d\n", check_bit(dmg_regs[0], 5));
	printf("SPRITE SIZE: 8x%d ", check_bit(dmg_regs[0], 2) ? 16 : 8);
	printf("BG MAP: 0x%04x ", check_bit(dmg_regs[0], 3) ? 0x9C00 : 0x9800);
	printf("WINDOW MAP: 0x%04x ", check_bit(dmg_regs[0], 6) ? 0x9C00 : 0x9800);
	printf("TILES: 0x%04x", check_bit(dmg_regs[0], 4) ? 0x8000 : 0x8800);

	printf("\n\ncgb DMA src: 0x%04x\tdst: 0x%04x\tstatus+len: 0x%02x", (cgb_regs[0] << 8) | cgb_regs[1], (cgb_regs[2] << 8) | cgb_regs[3], cgb_regs[4]);
	printf("\nvram bank: %d\n", cgb_regs[7]);
}

void Debugger::setup_entry_point()
{
	insert_breakpoint(0x100);
}

void Debugger::attach_mmu(function<u8(u16, u32)> read_byte, function<void(u16, u8, u32)> write_byte)
{
	read_byte_callback = read_byte;
	write_byte_callback = write_byte;
}

void Debugger::check_memory_access(u16 adress, u8 value)
{
	if (!memory_watches.empty() && (memory_watches.find(adress) != memory_watches.end()))
	{
		memory_changed = true;
		new_val = value;
		change_adress = adress;
	}
}

void Debugger::check_mmu()
{
	if (memory_changed)
	{
		memory_changed = false;
		enter_memory_trap();
		wait_for_user();
	}
}

void Debugger::step()
{
	current_pc = *pc;

	if (next_instruction || is_breakpoint() || (step_over && *pc == step_over_adress))
	{
		enter_trap();
		wait_for_user();
	}
}

void Debugger::after_vblank()
{
	if (vblanks_left == 1) 
		next_instruction = true; 
	
	if (vblanks_left > 0)
		--vblanks_left;
}

const char* Debugger::dispatch_opcode(u8 opcode, u8 byte_1)
{
	static const char* opcodes[] = {
		"NOP", "LD BC,0x%X", "LD (BC),A", "INC BC",
		"INC B","DEC B", "LD B,0x%X", "RLCA",
		"LD (0x%X),SP", "ADD HL,BC", "LD A,(BC)", "DEC BC",
		"INC C","DEC C", "LD C,0x%X", "RRCA",

		"STOP", "LD DE,0x%X", "LD (DE),A", "INC DE",
		"INC D", "DEC D", "LD D,0x%X", "RLA",
		"JR 0x%X", "ADD HL,DE", "LD A,(DE)", "DEC DE",
		"INC E", "DEC E", "LD E,0x%X", "RRA",

		"JR NZ,0x%X", "LD HL,0x%X", "LD (HL++),A", "INC HL",
		"INC H", "DEC H", "LD H,0x%X", "DAA",
		"JR Z,0x%X", "ADD HL,HL", "LD A,(HL++)", "DEC HL",
		"INC L", "DEC L", "LD L,0x%X", "CPL",

		"JR NC,0x%X", "LD SP,0x%X", "LD (HL--),A", "INC SP",
		"INC (HL)", "DEC (HL)", "LD (HL),0x%X", "SCF",
		"JR C,0x%X", "ADD HL,SP","LD A,(HL--)", "DEC SP",
		"INC A", "DEC A", "LD A,0x%X", "CCF",

		"LD B,B", "LD B,C", "LD B,D", "LD B,E",
		"LD B,H","LD B,L", "LD B,(HL)", "LD B,A",
		"LD C,B", "LD C,C", "LD C,D", "LD C,E",
		"LD C,H", "LD C,L", "LD C,(HL)", "LD C,A",

		"LD D,B", "LD D,C", "LD D,D", "LD D,E",
		"LD D,H","LD D,L", "LD D,(HL)", "LD D,A",
		"LD E,B", "LD E,C", "LD E,D", "LD E,E",
		"LD E,H", "LD E,L", "LD E,(HL)", "LD E,A",

		"LD H,B", "LD H,C", "LD H,D", "LD H,E",
		"LD H,H","LD H,L", "LD H,(HL)", "LD H,A",
		"LD L,B", "LD L,C", "LD L,D", "LD L,E",
		"LD L,H", "LD L,L", "LD L,(HL)", "LD L,A",

		"LD (HL),B", "LD (HL),C", "LD (HL),D", "LD (HL),E",
		"LD (HL),H","LD (HL),L", "HALT", "LD (HL),A",
		"LD A,B", "LD A,C", "LD A,D", "LD A,E",
		"LD A,H", "LD A,L", "LD A,(HL)", "LD A,A",

		"ADD A,B", "ADD A,C", "ADD A,D", "ADD A,E",
		"ADD A,H", "ADD A,L", "ADD A,(HL)", "ADD A,A",
		"ADC A,B", "ADC A,C", "ADC A,D", "ADC A,E",
		"ADC A,H", "ADC A,L", "ADC A,(HL)", "ADC A,A",

		"SUB A,B", "SUB A,C", "SUB A,D", "SUB A,E",
		"SUB A,H", "SUB A,L", "SUB A,(HL)", "SUB A,A",
		"SBC A,B", "SBC A,C", "SBC A,D", "SBC A,E",
		"SBC A,H", "SBC A,L", "SBC A,(HL)", "SBC A,A",

		"AND A,B", "AND A,C", "AND A,D", "AND A,E",
		"AND A,H", "AND A,L", "AND A,(HL)", "AND A,A",
		"XOR A,B", "XOR A,C", "XOR A,D", "XOR A,E",
		"XOR A,H", "XOR A,L", "XOR A,(HL)", "XOR A,A",

		"OR A,B", "OR A,C", "OR A,D", "OR A,E",
		"OR A,H", "OR A,L", "OR A,(HL)", "OR A,A",
		"CP A,B", "CP A,C", "CP A,D", "CP A,E",
		"CP A,H", "CP A,L", "CP A,(HL)", "CP A,A",

		"RET NZ", "POP BC", "JP NZ,0x%X", "JP 0x%X",
		"CALL NZ,0x%X", "PUSH BC", "ADD A,0x%X", "RST 00H",
		"RET Z", "RET", "JP Z,0x%X", "EXT_INSTRUCTION",
		"CALL Z,0x%X", "CALL 0x%X", "ADC A,0x%X", "RST 08H",

		"RET NC", "POP DE", "JP NC,0x%X", "NONE",
		"CALL NC,0x%X", "PUSH DE", "SUB 0x%X", "RST 10H",
		"RET C", "RETI", "JP C,0x%X", "NONE",
		"CALL C,0x%X", "NONE", "SBC A,0x%X", "RST 18H",

		"LDH (0x%X), A", "POP HL", "LD (C),A", "NONE",
		"NONE", "PUSH HL", "AND 0x%X", "RST 20H",
		"ADD SP,0x%X", "JP (HL)", "LD (0x%X),A", "NONE",
		"NONE", "NONE", "XOR 0x%X", "RST 28H",

		"LDH A,(0x%X)", "POP AF", "LD A,(C)", "DI",
		"NONE", "PUSH AF", "OR 0x%X", "RST 30H",
		"LD HL,SP+0x%X", "LD SP,HL", "LD A,(0x%X)", "EI",
		"NONE", "NONE", "CP 0x%X", "RST 38H"

	};

	static const char* ext_opcodes[] = {
		"RLC B", "RLC C", "RLC D", "RLC E",
		"RLC H", "RLC L", "RLC (HL)", "RLC A",

		"RRC B", "RRC C", "RRC D", "RRC E",
		"RRC H", "RRC L", "RRC (HL)", "RRC A",

		"RL B", "RL C", "RL D", "RL E",
		"RL H", "RL L", "RL (HL)", "RL A",

		"RR B", "RR C", "RR D", "RR E",
		"RR H", "RR L", "RR (HL)", "RR A",

		"SLA B", "SLA C", "SLA D", "SLA E",
		"SLA H", "SLA L", "SLA (HL)", "SLA A",

		"SRA B", "SRA C", "SRA D", "SRA E",
		"SRA H", "SRA L", "SRA (HL)", "SRA A",

		"SWAP B", "SWAP C", "SWAP D", "SWAP E",
		"SWAP H", "SWAP L", "SWAP (HL)", "SWAP A",

		"SRL B", "SRL C", "SRL D", "SRL E",
		"SRL H", "SRL L", "SRL (HL)", "SRL A",

		"BIT 0,B", "BIT 0,C", "BIT 0,D", "BIT 0,E",
		"BIT 0,H", "BIT 0,L", "BIT 0,(HL)", "BIT 0,A",
		"BIT 1,B", "BIT 1,C", "BIT 1,D", "BIT 1,E",
		"BIT 1,H", "BIT 1,L", "BIT 1,(HL)", "BIT 1,A",
		"BIT 2,B", "BIT 2,C", "BIT 2,D", "BIT 2,E",
		"BIT 2,H", "BIT 2,L", "BIT 2,(HL)", "BIT 2,A",
		"BIT 3,B", "BIT 3,C", "BIT 3,D", "BIT 3,E",
		"BIT 3,H", "BIT 3,L", "BIT 3,(HL)", "BIT 3,A",
		"BIT 4,B", "BIT 4,C", "BIT 4,D", "BIT 4,E",
		"BIT 4,H", "BIT 4,L", "BIT 4,(HL)", "BIT 4,A",
		"BIT 5,B", "BIT 5,C", "BIT 5,D", "BIT 5,E",
		"BIT 5,H", "BIT 5,L", "BIT 5,(HL)", "BIT 5,A",
		"BIT 6,B", "BIT 6,C", "BIT 6,D", "BIT 6,E",
		"BIT 6,H", "BIT 6,L", "BIT 6,(HL)", "BIT 6,A",
		"BIT 7,B", "BIT 7,C", "BIT 7,D", "BIT 7,E",
		"BIT 7,H", "BIT 7,L", "BIT 7,(HL)", "BIT 7,A",

		"RES 0,B", "RES 0,C", "RES 0,D", "RES 0,E",
		"RES 0,H", "RES 0,L", "RES 0,(HL)", "RES 0,A",
		"RES 1,B", "RES 1,C", "RES 1,D", "RES 1,E",
		"RES 1,H", "RES 1,L", "RES 1,(HL)", "RES 1,A",
		"RES 2,B", "RES 2,C", "RES 2,D", "RES 2,E",
		"RES 2,H", "RES 2,L", "RES 2,(HL)", "RES 2,A",
		"RES 3,B", "RES 3,C", "RES 3,D", "RES 3,E",
		"RES 3,H", "RES 3,L", "RES 3,(HL)", "RES 3,A",
		"RES 4,B", "RES 4,C", "RES 4,D", "RES 4,E",
		"RES 4,H", "RES 4,L", "RES 4,(HL)", "RES 4,A",
		"RES 5,B", "RES 5,C", "RES 5,D", "RES 5,E",
		"RES 5,H", "RES 5,L", "RES 5,(HL)", "RES 5,A",
		"RES 6,B", "RES 6,C", "RES 6,D", "RES 6,E",
		"RES 6,H", "RES 6,L", "RES 6,(HL)", "RES 6,A",
		"RES 7,B", "RES 7,C", "RES 7,D", "RES 7,E",
		"RES 7,H", "RES 7,L", "RES 7,(HL)", "RES 7,A",

		"SET 0,B", "SET 0,C", "SET 0,D", "SET 0,E",
		"SET 0,H", "SET 0,L", "SET 0,(HL)", "SET 0,A",
		"SET 1,B", "SET 1,C", "SET 1,D", "SET 1,E",
		"SET 1,H", "SET 1,L", "SET 1,(HL)", "SET 1,A",
		"SET 2,B", "SET 2,C", "SET 2,D", "SET 2,E",
		"SET 2,H", "SET 2,L", "SET 2,(HL)", "SET 2,A",
		"SET 3,B", "SET 3,C", "SET 3,D", "SET 3,E",
		"SET 3,H", "SET 3,L", "SET 3,(HL)", "SET 3,A",
		"SET 4,B", "SET 4,C", "SET 4,D", "SET 4,E",
		"SET 4,H", "SET 4,L", "SET 4,(HL)", "SET 4,A",
		"SET 5,B", "SET 5,C", "SET 5,D", "SET 5,E",
		"SET 5,H", "SET 5,L", "SET 5,(HL)", "SET 5,A",
		"SET 6,B", "SET 6,C", "SET 6,D", "SET 6,E",
		"SET 6,H", "SET 6,L", "SET 6,(HL)", "SET 6,A",
		"SET 7,B", "SET 7,C", "SET 7,D", "SET 7,E",
		"SET 7,H", "SET 7,L", "SET 7,(HL)", "SET 7,A" };

	if (opcode != 0xCB)
		return opcodes[opcode];

	else
		return ext_opcodes[byte_1];
}

u8 Debugger::get_opcode_bytes(u8 opcode)
{
	static const u8 opcodes_length[] = {
		1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
		2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,3,3,3,1,2,1,1,1,3,2,3,3,2,1,
		1,1,3,1,3,1,2,1,1,1,3,1,3,1,2,1,
		2,1,2,1,1,1,2,1,2,1,3,1,1,1,2,1,
		2,1,2,1,1,1,2,1,2,1,3,1,1,1,2,1 };

	return opcodes_length[opcode];
}
