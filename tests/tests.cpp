#include <cpputils/cpputils.h>
#include <cpputils/asyncio/coroutine.h>
#include <chrono>

using namespace std::chrono_literals;
using namespace cpputils::asyncio;

// Coroutine function
coroutine<void> coroutine_func()
{
	// Testing hello world
	LOG_DEBUG("Hello, World!");
	co_await 1s;
	LOG_DEBUG("Hello, World! After 1s");
	co_await 5s;
	// Testing math
	LOG_DEBUG("add(1, 2) = {}", add(1, 2));
	LOG_DEBUG("sub(1, 2) = {}", sub(1, 2));
	LOG_DEBUG("mul(1, 2) = {}", mul(1, 2));
	LOG_DEBUG("divi(1, 2) = {}", divi(1, 2));

	co_await 2s;

	LOG_DEBUG("After 2s");
	// Testing config setting
	cpputils::Debug::get_global_logger()->set_config(cpputils::log_level::INFO);
	// Getting new logger
	auto logger = cpputils::Debug::get_logger("Cool Name");

	co_await 1s;
	// Calling hello world from logger
	LOGGER_LOG_INFO(logger, "Hello World From Logger: {}", logger->name());
	// This should not print
	LOGGER_LOG_DEBUG(logger, "Logging Injection {} \\{{}\\} {}", 5, 12.05, "Meee");
}

int main(int argc, char **argv)
{
	run_sync(coroutine_func());
	return 0;
}