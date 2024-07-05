#pragma once
#include <cpputils/cpputils_api.h>
#include <cpputils/string.h>
#include <cpputils/memory.h>
#include <cpputils/collections.h>
#include <cpputils/chrono.h>
#include <functional>

namespace cpputils
{
	// Forward declare classes
	class CPPUTILS_API Debug;
	struct CPPUTILS_API log_record;
	class CPPUTILS_API logger;
	class CPPUTILS_API log_handler;
	enum class CPPUTILS_API log_level;

	// Enum class log_level
	enum class log_level
	{
		DEBUG = 0,
		INFO = 1,
		WARNING = 2,
		ERROR = 3,
		SEVERE = 4
	};

	// Template specialization for log_level
	template <>
	inline string to_string(const log_level &value)
	{
		switch (value)
		{
		case log_level::DEBUG:
			return "DEBUG";
		case log_level::INFO:
			return "INFO";
		case log_level::WARNING:
			return "WARNING";
		case log_level::ERROR:
			return "ERROR";
		case log_level::SEVERE:
			return "SEVERE";
		default:
			return "UNKNOWN";
		}
	}

	// Class log_record
	struct log_record
	{
		// Log level
		log_level level;

		// Log message
		string message;

		// Log timestamp
		cpputils::chrono::time_point timestamp;

		// Log context
		string context;
	};

	// Class log_handler
	class log_handler
	{
	public:
		// Virtual destructor
		virtual ~log_handler() = default;

		// Virtual log method
		virtual void log(logger *logger, const log_record &record) = 0;

		// Console handler
		static ref<log_handler> console_handler();

		// Custom logger handler
		static ref<log_handler> from_custom_logger(std::function<void(logger *, const log_record &)> log_function);
	};

	// Class logger
	class logger
	{
	private:
		// Log handlers
		array_list<ref<log_handler>> handlers;
		string m_name;
		log_level config;

	public:
		// Default constructor
		logger(const string &name) : m_name(name), config(log_level::DEBUG) {}

		// Destructor
		~logger();

		// Add log handler
		void add_handler(ref<log_handler> handler);

		// Remove log handler
		void remove_handler(ref<log_handler> handler);

		// Log method
		virtual void log(const log_record &record);

		// Name method
		const string &name() const { return m_name; }

		// Config
		const log_level get_config() const { return config; }

		virtual void set_config(log_level config)
		{
			this->config = config;
		}

		// Log level methods
		template <typename... Args>
		void debug(const string &context, const string &message, Args... args)
		{
			log({log_level::DEBUG, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		void info(const string &context, const string &message, Args... args)
		{
			log({log_level::INFO, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		void warning(const string &context, const string &message, Args... args)
		{
			log({log_level::WARNING, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		void error(const string &context, const string &message, Args... args)
		{
			log({log_level::ERROR, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		void severe(const string &context, const string &message, Args... args)
		{
			log({log_level::SEVERE, format(message, args...), cpputils::chrono::now(), context});
		}
	};

	// Class debug
	class Debug
	{
		// Log message: [logger:level @ timestamp]: context : message
	public:
		// Loggers
		static ref<logger> get_global_logger();
		static ref<logger> get_logger(const string &name);
		// Log methods to global logger
		static void log(const log_record &record);

		// Log level methods
		template <typename... Args>
		static void debug(const string &context, const string &message, Args... args)
		{
			log({log_level::DEBUG, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		static void info(const string &context, const string &message, Args... args)
		{
			log({log_level::INFO, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		static void warning(const string &context, const string &message, Args... args)
		{
			log({log_level::WARNING, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		static void error(const string &context, const string &message, Args... args)
		{
			log({log_level::ERROR, format(message, args...), cpputils::chrono::now(), context});
		}

		template <typename... Args>
		static void severe(const string &context, const string &message, Args... args)
		{
			log({log_level::SEVERE, format(message, args...), cpputils::chrono::now(), context});
		}
	};

	// Define Logger macros
	inline string line_file_details(const char *file, const int line)
	{
		return "file " + string(file) + ", line:" + to_string(line);
	}

#define LOG_DEBUG(message, ...) cpputils::Debug::debug(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) cpputils::Debug::info(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) cpputils::Debug::warning(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) cpputils::Debug::error(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)

#define LOGGER_LOG_DEBUG(logger, message, ...) logger->debug(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOGGER_LOG_INFO(logger, message, ...) logger->info(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOGGER_LOG_WARNING(logger, message, ...) logger->warning(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
#define LOGGER_LOG_ERROR(logger, message, ...) logger->error(cpputils::line_file_details(__FILE__, __LINE__), message, ##__VA_ARGS__)
}