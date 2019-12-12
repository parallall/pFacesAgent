#include "pfaces-agent-helper.h"
#include "Logger.h"

#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/json.h>

#include <locale>
#include <codecvt>
#include <string>

// gets a value from the JSON string using a key
std::string 
pFacesAgentHelper::getJSONItemValue(const std::string& JSONstr, const std::string& key) {
	
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

// changes a value, using a key, in the JSON string and returning the updated JSON string
std::string
pFacesAgentHelper::updateJSONItemValue(const std::string& JSONstr, const std::string& key, const std::string& new_value) {

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

bool 
pFacesAgentHelper::isLoginNew(const std::string& userLoginJSONstr) {
	// extract premissiton value
	std::string premission_value =
		pFacesAgentHelper::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION);

	

	// check it
	if (premission_value == std::string(PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_requested))
		return true;
	else
		return false;
}

bool 
pFacesAgentHelper::isLoginPermitted(const std::string& userLoginJSONstr) {
	
	std::string user_id =
		pFacesAgentHelper::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_ID);

	std::string user_key =
		pFacesAgentHelper::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_KEY);

	// todo: implement a login logic by connecting to a login server
	if (user_id == user_id && user_key == user_key)
		return true;
	else
		return false; /*not happening*/
}

std::string 
pFacesAgentHelper::setLoginInfo(const std::string& userLoginJSONstr, const std::string& loginUrl, const std::string& loginPort) {

	std::string ret = userLoginJSONstr;
	ret = pFacesAgentHelper::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_LOGIN_URL, loginUrl);
	ret = pFacesAgentHelper::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_LOGIN_PORT, loginPort);
	return ret;
}

std::string 
pFacesAgentHelper::setLoginDone(const std::string& userLoginJSONstr, bool success, const std::string& msg) {
	std::string ret = userLoginJSONstr;
	if(success)
		ret = pFacesAgentHelper::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION, 
			PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_granted);
	else
		ret = pFacesAgentHelper::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION,
			PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_denied);
	
	ret = pFacesAgentHelper::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_INFO_MESSAGE, msg);
	
	return ret;
}

std::vector<std::pair<std::string, std::string>>
pFacesAgentHelper::initializeUserDictionary(const std::string& user_id) {

	std::vector<std::pair<std::string, std::string>> keyValuePairs;

	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_INFO, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_AGENT_VERSION, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_PFACES_VERSION, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_AGENT_STATUS, PFACES_AGENT_USER_DICT_AGENT_STATUS_busy));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_PROJECT_LIST, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_JOBS_LIST, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_COMPILE, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_UPDATE, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_KILL, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_ACTIVITY_LOG, ""));

	return keyValuePairs;
}