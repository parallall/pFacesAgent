#include <codecvt>
#include <thread>
#include <mutex>

#include "pfaces-agent-utils.h"

#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/json.h>



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
std::string pfacesAgentUtils::exec_blocking(std::string cmd) {
	char buffer[1024];
	std::string result = "";
	FILE* pipe = POPEN(cmd.c_str(), "r");

	if (!pipe)
		throw std::runtime_error("exec: popen() failed!");

	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	}
	catch (...) {
		PCLOSE(pipe);
		throw;
	}
	PCLOSE(pipe);
	return result;
}

// execute a command asynchronously and return a pipe pointer
void pfacesAgentUtils::exec_non_blocking_thread_ready(
	const std::string& cmd, 
	std::shared_ptr<std::string> result, 
	std::shared_ptr<bool> done_notifier,
	std::shared_ptr<bool> kill_signal,
	std::shared_ptr<std::mutex> thread_mutex) {

	char buffer[1024];
	FILE* pipe = POPEN(cmd.c_str(), "r");

	if (!pipe)
		throw std::runtime_error("exec: popen() failed!");

	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL && !(*kill_signal)) {
			(*thread_mutex).lock();
			*result += buffer;
			(*thread_mutex).unlock();
		}
	}
	catch (...) {
		PCLOSE(pipe);
		throw;
	}
	PCLOSE(pipe);
	(*thread_mutex).lock();
	*done_notifier = true;
	(*thread_mutex).unlock();
}