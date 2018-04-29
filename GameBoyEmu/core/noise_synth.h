#pragma once


class NoiseSynth
{
	private:
		u32 length_counter = 0;
		u32 envelope_counter = 0;
		u32 envelope_load = 0;
		u32 timer = 0;
		u32 current_divisor = 0;
		u32 clock_shift = 0;
		u32 volume_load = 0;
		u16 lfsr = 0;
		u8 volume = 0;
		u8 out_vol = 0;

		bool enabled = false;
		bool envelope_asc = false;
		bool length_enabled = false;
		bool envelope_enabled = false;
		bool width_mode = false;
		bool dac_enabled = false;

		void start_playing();

	public:
		NoiseSynth();

		bool is_enabled() const;
		void reset();
		void update_length();		
		void update_envelope();
		void step(u32 cycles, u8* sample_buffer = nullptr);
		
		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value, u32 seq_frame);

		u8 get_volume() const;

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};