#ifndef PFACES_AGENT_HELPER_H
#define PFACES_AGENT_HELPER_H

#include <iostream>
#include <string>
#include <vector>

#include "pfacesRemoteInterface.h"

class AgentConfigs {
public:
	std::string id;
	int listen_port;
	std::string device_mask;
	std::string device_ids_list;
	bool device_abuse;
};

// Login Dictionary: Keys/values for the submitted JSON object
#define PFACES_AGENT_LOGIN_DICT_USER_ID "user_id"
#define PFACES_AGENT_LOGIN_DICT_USER_KEY "user_key"
#define PFACES_AGENT_LOGIN_DICT_USER_CLIENT_VERSION "client_version"
#define PFACES_AGENT_LOGIN_DICT_USER_LOGIN_REQUEST_TIME_VERSION "login_request_time"
#define PFACES_AGENT_LOGIN_DICT_USER_LOGIN_TIME_VERSION "login_time"
#define PFACES_AGENT_LOGIN_DICT_USER_LOGIN_URL "login_url"
#define PFACES_AGENT_LOGIN_DICT_USER_LOGIN_PORT "login_port"
#define PFACES_AGENT_LOGIN_DICT_USER_PERMISSION "permission"
#define PFACES_AGENT_LOGIN_DICT_USER_INFO_MESSAGE "info_message"
#define PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_requested "requested"
#define PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_granted "granted"
#define PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_denied "denied"

// User Dictionary: Main keys
#define PFACES_AGENT_USER_DICT_USER_ID "user_id"
#define PFACES_AGENT_USER_DICT_INFO "info"
#define PFACES_AGENT_USER_DICT_AGENT_VERSION "pfaces_agent_version"
#define PFACES_AGENT_USER_DICT_PFACES_VERSION "pfaces_version"
#define PFACES_AGENT_USER_DICT_AGENT_STATUS "agent_status"
#define PFACES_AGENT_USER_DICT_DEVICE_LIST "devices_list"
#define PFACES_AGENT_USER_DICT_PROJECT_LIST "projects_list"
#define PFACES_AGENT_USER_DICT_JOBS_LIST "jobs_list"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_COMPILE "cr_compile"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_UPLOAD "cr_upload"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN "cr_run"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_KILL "cr_kill"
#define PFACES_AGENT_USER_DICT_ACTIVITY_LOG "activity_log"

// User Dictionary: Agent status possible values
#define PFACES_AGENT_USER_DICT_AGENT_STATUS_busy "busy"
#define PFACES_AGENT_USER_DICT_AGENT_STATUS_ready "ready"

// User Dictionary: List-of-devices JSON keys
#define PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_ID_JSON_KEY "device_id"
#define PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_NAME_JSON_KEY "device_name"
#define PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_TYPE_JSON_KEY "device_type"
#define PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_STATUS_JSON_KEY "device_status"

// User Dictionary: Any command request JSON keys
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY "request_status"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_TIME_JSON_KEY "request_time"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY "request_option"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_MESSAGE_JSON_KEY "message"

// User Dictionary: Request Status values
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_submitted "submitted"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_processing "processing"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_done "done"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_error "error"

// User Dictionary: Upload Request: Options
#define PFACES_AGENT_USER_DICT_PROJECT_UPLOAD_PROJECT_name "project_update_project_name"
#define PFACES_AGENT_USER_DICT_PROJECT_UPLOAD_PROJECT_blob "project_update_project_blob"

// User Dictionary: Rubn Request: Options
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_PROJECT_name "project_run_project_name"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_DEVICE_id "project_run_device_id"

// User Dictionary: List-of-Projects JSON keys
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_NAME_JSON_KEY "project_name"
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_DETAILS_JSON_KEY "project_details"
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_LAST_UPDATE_TIME_JSON_KEY "project_last_update_time"

// User Dictionary: List-of-Jobs JSON keys
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_ID_JSON_KEY "jobs_id"
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_STATUS_JSON_KEY "jobs_status"
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_DETAILS_JSON_KEY "jobs_details"

// User Dictionary: Activity-Log JSON keys
#define PFACES_AGENT_USER_DICT_ACTIVITY_ID_JSON_KEY "activity_id"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_TIME_JSON_KEY "activity_request_time"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_DETAILS_JSON_KEY "activity_request_details"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_RESULT_JSON_KEY "activity_request_result"


class pFacesAgentHelper {
public:

	// types of command requests
	enum class COMMAND_REQUESTS {
		STATUS,
		UPLOAD,
		COMPILE,
		UPDATE,
		RUN,
		KILL,
		NONE
	};

	// General
	static std::string getJSONItemValue(const std::string& JSONstr, const std::string& key);
	static std::string getSubJSONItemValue(const std::string& JSONstr, const std::string& array_key, const std::string& array_item_key);
	static std::string updateJSONItemValue(const std::string& JSONstr, const std::string& key, const std::string& new_value);

	// Login Management
	static bool isLoginNew(const std::string &userLoginJSONstr);	
	static bool isLoginPermitted(const std::string& userLoginJSONstr);
	static std::string setLoginInfo(const std::string& userLoginJSONstr, const std::string& loginUrl, const std::string& loginPort);
	static std::string setLoginDone(const std::string &userLoginJSONstr, bool success, const std::string &msg);

	// activity log
	static std::string addToActivityLog(const std::string& oldActivityLogJSON, const std::string& newLogStatus);

	// User management
	static std::vector<std::pair<std::string, std::string>> initializeUserDictionary(const std::string& user_id, const AgentConfigs& configs);

	// CR process
	static std::string commandRequestToString(COMMAND_REQUESTS cr_type);
	static size_t countPendingCommandRequest(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
	static std::pair<COMMAND_REQUESTS, bool> processNextPendingCommandRequest(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);

	// CR processing functions
	static void processCRUpload(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
	static void processCRCompile(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
	static void processCRUpdate(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
	static void processCRRun(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
	static void processCRKill(const std::shared_ptr<pfacesRESTfullDictionaryServer>& userDictionary);
};

#endif