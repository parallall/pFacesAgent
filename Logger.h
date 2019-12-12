#ifndef PFACES_AGENT_LOGGER
#define PFACES_AGENT_LOGGER

#include <string>

class Logger {
public:
	static
		void log(const std::string& from, const std::string& msg);
};

#endif