#include <codecvt>
#include <thread>
#include <mutex>

#include "pfaces-agent-utils.h"

#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/json.h>


#define MAX_APP_PATH_SIZE 256
std::string pfacesAgentUtils::getApplicationDirectory() {
	char cCurrentPath[MAX_APP_PATH_SIZE];
	int cnt;
#ifdef _MSC_VER
	cnt = GetModuleFileNameA(NULL, cCurrentPath, MAX_APP_PATH_SIZE);
#elif __APPLE__
	uint32_t size = MAX_APP_PATH_SIZE;
	cnt = _NSGetExecutablePath(cCurrentPath, &size);
#else
	cnt = readlink("/proc/self/exe", cCurrentPath, MAX_APP_PATH_SIZE);
#endif
	if (cnt < 0 || cnt >= MAX_APP_PATH_SIZE) {
		throw std::runtime_error(
			std::string("pfacesUtils::getApplicationDir: Failed to call the OS-API function to get Current Directory. ") +
			std::string("cnt=") +
			std::to_string(cnt)
		);
	}

	std::string ret(cCurrentPath);
	ret = ret.substr(0, ret.find_last_of(PATH_DELIMITER));

	char splitter_char = std::string(PATH_DELIMITER).c_str()[0];
	if (ret.c_str()[ret.size() - 1] != splitter_char)
		ret.append(1, splitter_char);

	return ret;
}


// gets a value from the JSON string using a key
std::string
pfacesAgentUtils::getJSONItemValue(const std::string& JSONstr, const std::string& key) {

	// prepare
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring JSONstr_wide = converter.from_bytes(JSONstr);
	std::wstring key_wide = converter.from_bytes(key);

	try {
		// parse
		web::json::value jobj = web::json::value::parse(JSONstr_wide);

		// extract & return 
		std::wstring value_wide = jobj.at(key_wide).as_string();
		return converter.to_bytes(value_wide);
	}
	catch (...) {
		return "";
	}
}

std::string pfacesAgentUtils::getSubJSONItemValue(
	const std::string& JSONstr,
	const std::string& array_key,
	const std::string& array_item_key) {

	// prepare
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring JSONstr_wide = converter.from_bytes(JSONstr);
	std::wstring array_key_wide = converter.from_bytes(array_key);
	std::wstring array_item_key_wide = converter.from_bytes(array_item_key);

	try {
		// parse
		web::json::value jobj = web::json::value::parse(JSONstr_wide);

		// extract the array
		web::json::value jArrayObj = jobj.at(array_key_wide);

		// extract & return the item in the array
		std::wstring value_wide = jArrayObj.at(array_item_key_wide).as_string();
		return converter.to_bytes(value_wide);
	}
	catch (...) {
		return "";
	}
}

// changes a value, using a key, in the JSON string and returning the updated JSON string
std::string
pfacesAgentUtils::updateJSONItemValue(
	const std::string& JSONstr, const std::string& key, const std::string& new_value) {

	// prepare
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring JSONstr_wide = converter.from_bytes(JSONstr);
	std::wstring key_wide = converter.from_bytes(key);
	std::wstring new_value_wide = converter.from_bytes(new_value);

	try {
		// parse, update and return 
		web::json::value jobj = web::json::value::parse(JSONstr_wide);
		jobj[key_wide] = web::json::value::string(new_value_wide);
		std::wstring ret_wide = jobj.serialize();
		return converter.to_bytes(ret_wide);
	}
	catch (...) {
		return "";
	}
}
std::string pfacesAgentUtils::makeJSONTable(
	const std::vector<std::string>& table_head_keys, 
	const std::vector<std::vector<std::string>>& table_rows) {

	std::vector<web::json::value> jArrayRows;
	for (size_t i = 0; i < table_rows.size(); i++){
		web::json::value j_row;
		for (size_t j = 0; j < table_head_keys.size(); j++){
			std::string head_name = table_head_keys[j];
			j_row[pfacesUtils::string2wstring(head_name)] = 
				web::json::value::string(pfacesUtils::string2wstring(table_rows[i][j]));
		}
		jArrayRows.push_back(j_row);
	}
	web::json::value as_arr = web::json::value::array(jArrayRows);
	utility::string_t as_wstr = as_arr.serialize();
	auto as_str = pfacesUtils::wstring2string(as_wstr);
	return as_str;
}



void pfacesAgentUtils::make_directory(const std::string& path) {
	MAKE_DIR(path.c_str());
}

// discovering directories in a path
std::vector<std::string> pfacesAgentUtils::get_path_directories(const std::string& path) {
	std::vector<std::string> ret;
#ifdef _MSC_VER
	std::string all_contents = path + std::string("*");
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	if ((hFind = FindFirstFile(all_contents.c_str(), &FindFileData)) != INVALID_HANDLE_VALUE) {
		do {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::string fullname(FindFileData.cFileName);
				std::vector<std::string> splitted = pfacesUtils::strSplit(fullname, PATH_DELIMITER, false);
				if (splitted.size() >= 1) {
					std::string actual_name = splitted[splitted.size() - 1];
					if (actual_name != std::string(".") && actual_name != std::string(".."))
						ret.push_back(actual_name);
				}
			}
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
#else
#error To be implemented from https://stackoverflow.com/questions/5043403/listing-only-folders-in-directory using opendir() and readdir()
#endif
	return ret;
}


// execute a command in system
#define EXEC_BLOCKING_BUFFER_SIZE 1024
std::string pfacesAgentUtils::exec_blocking(std::string cmd) {
	char* buffer = (char*)malloc(EXEC_BLOCKING_BUFFER_SIZE);
	std::string result("");
	FILE* pipe = POPEN(cmd.c_str(), "r");

	if (!pipe)
		throw std::runtime_error("exec: popen() failed!");

	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += std::string(buffer);
			buffer[0] = 0;
		}
	}
	catch (...) {
		PCLOSE(pipe);
		free(buffer);
		throw;
	}
	PCLOSE(pipe);
	free(buffer);
	return result;
}

// execute a command asynchronously and return a pipe pointer
#define EXEC_NON_BLOCKING_BUFFER_SIZE 1024
void pfacesAgentUtils::exec_non_blocking_thread_ready(std::shared_ptr<non_blocking_thread_pack> control_pack) {

	char* buffer = (char*)malloc(EXEC_NON_BLOCKING_BUFFER_SIZE);
	FILE* pipe = POPEN(control_pack->launch_cmd.c_str(), "r");

	if (!pipe)
		throw std::runtime_error("exec: popen() failed!");

	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL && !control_pack->kill_signal) {
			std::lock_guard<std::mutex> lock(control_pack->thread_mutex);
			control_pack->output += std::string(buffer);
			buffer[0] = 0;
		}
	}
	catch (...) {
		PCLOSE(pipe);
		free(buffer);
		throw;
	}
	PCLOSE(pipe);
	free(buffer);
	std::lock_guard<std::mutex> lock(control_pack->thread_mutex);
	control_pack->done_notifier = true;
}