#include "square_synth.h"

void SquareSynth::update_sweep()
{
	--sweep_counter;

	if (sweep_counter == 0)
	{
		sweep_counter = sweep_load;

		if (sweep_counter == 0)
			sweep_counter = 8;

		if (sweep_enabled && sweep_load > 0)
		{

		}
	}
}

void SquareSynth::update_length()
{
	if (length_enabled)
	{
		--length_counter;

		if (length_counter == 0)
			enabled = false;
	}
}

void SquareSynth::update_envelope()
{
	--envelope_counter;

	if (envelope_enabled && envelope_counter == 0)
	{
		envelope_counter = start_envelope;

		if (envelope_counter == 0) //obscure behaviour
			envelope_counter = 8;

		if (envelope_asc && volume < 15)
			++volume;

		else if (!envelope_asc && volume > 0)
			--volume;

		if (volume == 0 || volume == 15)
			envelope_enabled = false;
	}
}

void SquareSynth::step(u32 cycles)
{
	static const u8 duty_lut[] = { 0x01, 0x81, 0x87, 0x7E };

	while (cycles > timer)
	{
		cycles -= timer;

		timer = (2048 - freq) * 4;
		duty_pos = (duty_pos + 1) % 8;

		out_vol = enabled && dac_enabled && check_bit(duty_lut[duty], duty_pos) ? volume : 0;
	}

	timer -= cycles;
}
