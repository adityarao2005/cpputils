#include <cpputils/cpputils.h>

int main(int argc, char **argv)
{
	std::cout << "Hello, World!" << std::endl;
	std::cout << "add(1, 2) = " << add(1, 2) << std::endl;
	std::cout << "sub(1, 2) = " << sub(1, 2) << std::endl;
	std::cout << "mul(1, 2) = " << mul(1, 2) << std::endl;
	std::cout << "divi(1, 2) = " << divi(1, 2) << std::endl;
	LOG_DEBUG("Hello, World!");
	auto logger = cpputils::Debug::get_logger("Cool Name");
	LOGGER_LOG_DEBUG(logger, "Hello World From Logger: Cool Name");
	LOGGER_LOG_DEBUG(logger, "Logging Injection {} \\{{}\\} {}", 5, 12.05, "Meee");
	std::cout << "hello world" << std::endl;
	return 0;
}