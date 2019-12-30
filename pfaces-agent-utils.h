#ifndef PFACES_AGENT_UTILS
#define PFACES_AGENT_UTILS

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "pfacesUtils.h"

#ifdef _MSC_VER
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>  

#define PATH_DELIMITER "\\"
#define MAKE_DIR(x) _mkdir(x)
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/stat.h>
#define PATH_DELIMITER "/"
#define MAKE_DIR(x) mkdir(x,????)
#define POPEN popen
#define PCLOSE pclose
#endif

enum class EXEC_STATUS {
	NO_DATA_YET,
	DATA_RECIEVED,
	PIPE_CLOSED
};

class pfacesAgentUtils{
public:
	// json
	static std::string getJSONItemValue(const std::string& JSONstr, const std::string& key);
	static std::string getSubJSONItemValue(const std::string& JSONstr, const std::string& array_key, const std::string& array_item_key);
	static std::string updateJSONItemValue(const std::string& JSONstr, const std::string& key, const std::string& new_value);
	static std::string makeJSONTable(const std::vector<std::string>& table_head_keys, const std::vector<std::vector<std::string>>& table_rows);

	// file sys
	static std::string getApplicationDirectory();
	static void make_directory(const std::string& path);
	static std::vector<std::string> get_path_directories(const std::string& path);

	// processes
	static std::string exec_blocking(std::string cmd);
	static void exec_non_blocking_thread_ready(
		const std::string& cmd,
		std::shared_ptr<std::string> result,
		std::shared_ptr<bool> done_notifier,
		std::shared_ptr<bool> kill_signal,
		std::shared_ptr<std::mutex> thread_mutex);
};

#endif // !PFACES_AGENT_UTILS
