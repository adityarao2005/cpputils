#include <cpputils/debug.h>
#include <iostream>
#include <sstream>

// Define namespace
using namespace cpputils;

// Define class logger
class console_log_handler;

// Define class console_log_handler for writing to console
class console_log_handler : public log_handler
{
public:
	// Log message
	void log(logger *logger, const log_record &record) override
	{

		// Log message: [logger:level @ timestamp]: context : message
		std::stringstream ss;
		ss << "[" << logger->name() << ":" << to_string(record.level) << " @ " << to_string(record.timestamp) << "]: " << record.context << " : " << record.message;
		std::cout << ss.str() << std::endl;
	}
};

// Define class log_handler
ref<log_handler> log_handler::console_handler()
{
	return make_ref<console_log_handler>();
}

// Define class log_handler
ref<log_handler> log_handler::from_custom_logger(std::function<void(logger *, const log_record &)> log_function)
{
	// Define class custom_log_handler
	class custom_log_handler : public log_handler
	{
	private:
		// Define log function
		std::function<void(logger *, const log_record &)> m_log_function;

	public:
		// Define custom log handler
		custom_log_handler(std::function<void(logger *, const log_record &)> log_function)
			: m_log_function(log_function)
		{
		}

		// Log message
		void log(logger *logger, const log_record &record) override
		{
			m_log_function(logger, record);
		}
	};

	// Return custom log handler
	return make_ref<custom_log_handler>(log_function);
}

// Define logger::add_handler method: adds to list
void logger::add_handler(ref<log_handler> handler)
{
	handlers.push_back(handler);
}

// Define logger::add_handler method: removes from list
void logger::remove_handler(ref<log_handler> handler)
{
	// finds the handler first
	auto it = std::find(handlers.begin(), handlers.end(), handler);
	// Removes from list
	if (it != handlers.end())
		handlers.erase(it);
}

// Logs the record
void logger::log(const log_record &record)
{
	if (this->get_config() > record.level)
		return;
	// Iterates through the handlers
	for (auto &handler : handlers)
	{
		handler->log(this, record);
	}
}

// Destructor
logger::~logger()
{
}

// Global logger
class global_logger : public logger
{
private:
	// Has list of other loggers
	array_list<ref<logger>> loggers;

public:
	// Default constructor
	global_logger() : logger("global")
	{
		// Adds console handler
		add_handler(log_handler::console_handler());
	}

	// Logs the record
	void log(const log_record &record) override
	{
		// Calls the parent log method
		logger::log(record);

		// Iterates through the loggers
		for (auto &logger : loggers)
		{
			logger->log(record);
		}
	}

	void set_config(log_level level) override
	{
		logger::set_config(level);

		// Iterates through the loggers
		for (auto &logger : loggers)
		{
			logger->set_config(level);
		}
	}

	// Gets the instance
	static ref<global_logger> get_instance()
	{
		// Returns the instance
		static ref<global_logger> instance = make_ref<global_logger>();
		return instance;
	}

	// Gets logger
	ref<logger> get_logger(const string &name)
	{
		// Finds the logger
		for (auto &logger : loggers)
		{
			if (logger->name() == name)
				return logger;
		}

		// Creates a logger
		ref<logger> m_logger = make_ref<logger>(name);
		m_logger->add_handler(log_handler::console_handler());
		m_logger->set_config(this->get_config());
		loggers.push_back(m_logger);
		return m_logger;
	}
};

// Gets the global logger
ref<logger> Debug::get_global_logger()
{
	return global_logger::get_instance();
}

// Gets the logger
ref<logger> Debug::get_logger(const string &name)
{
	if (name == "global")
		return get_global_logger();
	else
		return global_logger::get_instance()->get_logger(name);
}

// Logs the record
void Debug::log(const log_record &record)
{
	global_logger::get_instance()->log(record);
}