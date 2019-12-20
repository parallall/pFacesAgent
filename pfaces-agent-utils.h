#ifndef PFACES_AGENT_UTILS
#define PFACES_AGENT_UTILS

#include <iostream>
#include <string>
#include <vector>
#include "pfacesUtils.h"

#ifdef _MSC_VER
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>  

#define PATH_DELIMITER "\\"
#define USER_DATA_DIRECTORY "C:" PATH_DELIMITER "pfaces_user_data" PATH_DELIMITER
#define MAKE_DIR(x) _mkdir(x)
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/stat.h>
#define PATH_DELIMITER "/"
#define USER_DATA_DIRECTORY PATH_DELIMITER "pfaces_user_data" PATH_DELIMITER
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
	static void make_directory(const std::string& path);
	static std::vector<std::string> get_path_directories(const std::string& path);

	// processes
	static std::string exec_blocking(std::string cmd);
	static std::shared_ptr<FILE> exec_non_blocking(std::string cmd);
	static std::pair<std::string, EXEC_STATUS> non_blocking_exec_status(std::shared_ptr<FILE>& pipe);
	static void non_blocking_exec_kill(std::shared_ptr<FILE>& pipe);
};

#endif // !PFACES_AGENT_UTILS
