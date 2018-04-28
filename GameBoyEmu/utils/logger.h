#pragma once


class Logger
{
	private:
		std::ofstream log_file;

		Logger() {}
		~Logger() {}

	public:
		static Logger& get_instance()
		{
			static Logger inst;
			return inst;
		}

		void init(const char* file_name)
		{
			log_file.open(file_name);
		}

		void log(u64 cycles_from_start, const std::string& event, const std::string& msg)
		{
			log_file << "[" << cycles_from_start << "](" << event << ") " << msg << "\n";
		}
};

#ifdef ENABLE_LOGGER

#define INIT_LOGGER(log_file_name) Logger::get_instance().init(log_file_name);
#define DEBUG_LOG(msg) Logger::get_instance().log(internal_logger_local_counter, internal_logger_channel_name, msg);

#define LOG_CHANNEL(name) static const char internal_logger_channel_name[] = name; \
						  static u64 internal_logger_local_counter = 0;
#define LOG_ADD_CYCLES(cycles) internal_logger_local_counter += cycles;

#else

#define INIT_LOGGER(log_file_name)
#define DEBUG_LOG(msg)

#define LOG_CHANNEL(name)
#define LOG_ADD_CYCLES(cycles)

#endif