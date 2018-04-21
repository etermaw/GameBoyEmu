#include "core.h"

Core::Core() : cpu(mmu, ints), gpu(ints), timer(ints)
{
	mmu.register_chunk(0, 0x7FFF, cart.get_memory_controller());
	mmu.register_chunk(0x8000, 0x9FFF, &gpu); //vram
	mmu.register_chunk(0xA000, 0xBFFF, cart.get_memory_controller());
	mmu.register_chunk(0xC000, 0xFDFF, &ram);
	mmu.register_chunk(0xFE00, 0xFE9F, &gpu); //oam tables
	mmu.register_chunk(0xFF00, 0xFF00, &joypad);//input keys register
	mmu.register_chunk(0xFF01, 0xFF02, &tr); //TODO: test reader, there should be serial port
	mmu.register_chunk(0xFF04, 0xFF07, &timer);//timer controls
	mmu.register_chunk(0xFF0F, 0xFF0F, &ints);//interrupts flags
	mmu.register_chunk(0xFF10, 0xFF3F, &apu); //APU registers + wave RAM 
	mmu.register_chunk(0xFF40, 0xFF4B, &gpu); //gpu control regs
	mmu.register_chunk(0xFF4D, 0xFF4D, &speed); //CPU speed switch (CGB)
	mmu.register_chunk(0xFF4F, 0xFF4F, &gpu); //gpu vram bank reg (CGB)
	mmu.register_chunk(0xFF51, 0xFF55, &gpu); //gpu HDMA/GDMA regs (CGB)
	mmu.register_chunk(0xFF68, 0xFF6B, &gpu); //gpu color BGP/OBP regs (CGB)
	mmu.register_chunk(0xFF70, 0xFF70, &ram); //ram bank register (CGB)
	mmu.register_chunk(0xFF76, 0xFF77, &apu); //APU PCM registers (CGB)
	mmu.register_chunk(0xFF80, 0xFFFE, &ram); //high ram
	mmu.register_chunk(0xFFFF, 0xFFFF, &ints); //interrupts

	debugger.attach_mmu(make_function(&MMU::read_byte, &mmu), make_function(&MMU::write_byte, &mmu));
	debugger.attach_gpu(gpu.get_debug_func());

	cpu.attach_debugger(debugger.get_cpu());
	mmu.attach_debug_callback(make_function(&Debugger::check_memory_access, &debugger));
}

bool Core::load_cartrige(std::ifstream& rom_file, std::ifstream& ram_file, std::ifstream& rtc_file) 
{
	if (!cart.load_cartrige(rom_file, ram_file, rtc_file))
		return false;

	gpu.attach_dma_ptrs(cart.get_dma_controller(), &ram);

	bool enable_cgb = cart.is_cgb_ready();
	cpu.enable_cgb_mode(enable_cgb);
	gpu.enable_cgb_mode(enable_cgb);
	ram.enable_cgb_mode(enable_cgb);
	apu.enable_cgb_mode(enable_cgb);

	mmu.swap_chunk(0, 0x8000, cart.get_memory_controller()); //TODO: it`s inconsistent with register_chunk scheme of adressing
	mmu.swap_chunk(0xA000, 0xC000, cart.get_memory_controller());

	return true;
}

std::string Core::get_cart_name()
{
   return cart.get_name(); 
}

void Core::load_state(std::istream& load_stream)
{
	ints.deserialize(load_stream);
	//mmu

	cpu.deserialize(load_stream);
	gpu.deserialize(load_stream);
	apu.deserialize(load_stream);

	timer.deserialize(load_stream);
	cart.deserialize(load_stream);
	ram.deserialize(load_stream);
	//serial port
	joypad.deserialize(load_stream);
	speed.deserialize(load_stream);
}

void Core::save_state(std::ostream& save_stream)
{
	ints.serialize(save_stream);
	//mmu

	cpu.serialize(save_stream);
	gpu.serialize(save_stream);
	apu.serialize(save_stream);

	timer.serialize(save_stream);
	cart.serialize(save_stream);
	ram.serialize(save_stream);
	//serial port
	joypad.serialize(save_stream);
	speed.serialize(save_stream);
}

void Core::run_one_frame()
{
	//we have input already
	//save state is handled outside

	while (!gpu.is_entering_vblank()) //TODO: if someone turn off lcd, this loop may spin forever
	{
		u32 sync_cycles = 0;

		if (ints.is_any_raised())
		{
			cpu.unhalt();

			if (cpu.is_interrupt_enabled())
				sync_cycles = cpu.handle_interrupt();
		}

		debugger.step();

		sync_cycles += cpu.step(sync_cycles);
		sync_cycles += gpu.step(sync_cycles);
		apu.step(sync_cycles);
		timer.step(sync_cycles);

		gpu.set_speed(speed.double_speed);
		apu.set_speed(speed.double_speed);
	}

	draw_frame_callback(gpu.get_frame_buffer());
	gpu.clear_frame_buffer();
	debugger.after_vblank();
}

void Core::push_key(KEYS key)
{
	joypad.push_key(key);
}

void Core::release_key(KEYS key)
{
	joypad.release_key(key);
}

void Core::attach_callbacks(const external_callbacks& endpoints)
{
	cart.attach_endpoints(endpoints.save_ram, endpoints.save_rtc);
	apu.attach_endpoints(endpoints.swap_sample_buffer, endpoints.audio_control);

	draw_frame_callback = endpoints.draw_frame;
}
