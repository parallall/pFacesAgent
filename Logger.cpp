#include <iostream>
#include <mutex>

#include "Logger.h"
#include "pfacesIO.h"
#include "pfaces-agent-utils.h"

#define LOG_FILE (pfacesAgentUtils::getApplicationDirectory() + std::string("pfaces_agent.log"))
#define LOG(FROM, TEXT) pfacesFileIO::appendTextToFile(LOG_FILE, (std::string("[") + pfacesUtils::current_date() + std::string("] ") + std::string(FROM) + std::string(": ") + std::string(TEXT)))

std::mutex logOrganizer;
void Logger::log(const std::string& from, const std::string& msg) {
	logOrganizer.lock();
	std::string msg2 = msg + std::string("\n");
	LOG(from, msg2);
	logOrganizer.unlock();
}