#include <iostream>
#include <mutex>

#include "Logger.h"
#include "pfacesUtils.h"
#include "pfacesIO.h"

#define LOG_FILE "pfaces_agent.log"
#define LOG(FROM, TEXT) pfacesFileIO::appendTextToFile(std::string(LOG_FILE), (std::string(FROM) + std::string(" [") + pfacesUtils::current_date() + std::string("]: ") + std::string(TEXT)))

std::mutex logOrganizer;
void Logger::log(const std::string& from, const std::string& msg) {
	logOrganizer.lock();
	std::string msg2 = msg + std::string("\n");
	LOG(from, msg2);
	logOrganizer.unlock();
}