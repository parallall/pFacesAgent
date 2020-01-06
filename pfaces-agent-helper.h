#ifndef PFACES_AGENT_HELPER_H
#define PFACES_AGENT_HELPER_H

#include <iostream>
#include <string>
#include <vector>

#include "pfacesRemoteInterface.h"
#include "pfaces-job-manager.h"

class AgentConfigs {
public:
	std::string id;
	int listen_port;
	std::string user_data_directory;
	std::string device_mask;
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
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_BLOB_JSON_KEY "blob"

// User Dictionary: Request Status values
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_submitted "submitted"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_processing "processing"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_done "done"
#define PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_error "error"

// User Dictionary: Upload Request: Options
#define PFACES_AGENT_USER_DICT_PROJECT_UPLOAD_PROJECT_name "project_update_project_name"

// User Dictionary: Run Request: Options
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_PROJECT_name "project_run_project_name"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_DEVICE_id "project_run_device_id"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_KERNEL_name "project_run_kernel_name"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_KERNEL_dir "project_run_kernel_dir"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_CONFIG_path "project_run_config_path"
#define PFACES_AGENT_USER_DICT_PROJECT_RUN_extras "project_run_extras"

// User Dictionary: Kill Request: Options
#define PFACES_AGENT_USER_DICT_PROJECT_KILL_JOB_id "project_kill_job_id"

// User Dictionary: List-of-Projects JSON keys
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_NAME_JSON_KEY "project_name"
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_DETAILS_JSON_KEY "project_details"
#define PFACES_AGENT_USER_DICT_PROJECT_LIST_PROJECT_LAST_UPDATE_TIME_JSON_KEY "project_last_update_time"

// User Dictionary: List-of-Jobs JSON keys
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_ID_JSON_KEY "jobs_id"
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_ID_JSON_CMD "jobs_cmd"
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_STATUS_JSON_KEY "jobs_status"
#define PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_DETAILS_JSON_KEY "jobs_details"

// User Dictionary: Activity-Log JSON keys
#define PFACES_AGENT_USER_DICT_ACTIVITY_ID_JSON_KEY "activity_id"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_TIME_JSON_KEY "activity_request_time"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_DETAILS_JSON_KEY "activity_request_details"
#define PFACES_AGENT_USER_DICT_ACTIVITY_REQUEST_RESULT_JSON_KEY "activity_request_result"

class pFacesAgentUserContext {
public:
	AgentConfigs spAgentConfigs;
	std::shared_ptr<pfacesRESTfullDictionaryServer> spUserDictionary;
	std::shared_ptr<pfacesJobManager> spJobManager;
};

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
	static size_t countPendingCommandRequest(pFacesAgentUserContext& userContext);
	static std::pair<COMMAND_REQUESTS, bool> 
		processNextPendingCommandRequest(pFacesAgentUserContext& userContext);

	// CR processing functions
	static void processCRUpload(pFacesAgentUserContext& userContext);
	static void processCRCompile(pFacesAgentUserContext& userContext);
	static void processCRRun(pFacesAgentUserContext& userContext);
	static void processCRKill(pFacesAgentUserContext& userContext);
};

#endif