#include <cpputils/cpputils.h>

int main(int argc, char **argv)
{
	// Testing hello world
	LOG_DEBUG("Hello, World!");
	// Testing math
	LOG_DEBUG("add(1, 2) = {}", add(1, 2));
	LOG_DEBUG("sub(1, 2) = {}", sub(1, 2));
	LOG_DEBUG("mul(1, 2) = {}", mul(1, 2));
	LOG_DEBUG("divi(1, 2) = {}", divi(1, 2));

	// Testing config setting
	cpputils::Debug::get_global_logger()->set_config(cpputils::log_level::INFO);
	// Getting new logger
	auto logger = cpputils::Debug::get_logger("Cool Name");
	// Calling hello world from logger
	LOGGER_LOG_INFO(logger, "Hello World From Logger: {}", logger->name());
	// This should not print
	LOGGER_LOG_DEBUG(logger, "Logging Injection {} \\{{}\\} {}", 5, 12.05, "Meee");
	return 0;
}