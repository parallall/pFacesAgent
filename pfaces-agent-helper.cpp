#include <string>
#include <functional>
#include <iostream>
#include <fstream>

#include "pfaces-agent-utils.h"
#include "pfaces-agent-helper.h"
#include "Logger.h"
#include "base64.h"
#include "zipper.h"


// the next vectors are together-related and used for calling the 
// processor functions
std::vector<std::string> CRs_to_check = {
	PFACES_AGENT_USER_DICT_COMMAND_REQUEST_UPLOAD,
	PFACES_AGENT_USER_DICT_COMMAND_REQUEST_COMPILE,
	PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN,
	PFACES_AGENT_USER_DICT_COMMAND_REQUEST_KILL
};
std::vector<pFacesAgentHelper::COMMAND_REQUESTS> CR_Types = {
	pFacesAgentHelper::COMMAND_REQUESTS::UPLOAD,
	pFacesAgentHelper::COMMAND_REQUESTS::COMPILE,
	pFacesAgentHelper::COMMAND_REQUESTS::RUN,
	pFacesAgentHelper::COMMAND_REQUESTS::KILL
};
std::vector<std::function<void(pFacesAgentUserContext&)>> CR_processors = {
	pFacesAgentHelper::processCRUpload,
	pFacesAgentHelper::processCRCompile,
	pFacesAgentHelper::processCRRun,
	pFacesAgentHelper::processCRKill
};


// management stuff
std::string getpFacesVersion() {
	std::string cmd = std::string("pfaces -h -v0");
	std::string version_line = pfacesAgentUtils::exec_blocking(cmd);
	version_line = pfacesUtils::strReplaceAll(version_line, "\n", "");
	version_line = pfacesUtils::strReplaceAll(version_line, "\t", "");
	return version_line;
}
std::string getDevicesListJSON(std::string device_mask) {
	
	// call pFaces / get devices list
	std::string cmd = std::string("pfaces -") + device_mask + std::string(" -l -v0");
	std::string cmd_result = pfacesAgentUtils::exec_blocking(cmd);

	// split by lines
	std::vector<std::string> dev_lines = pfacesUtils::strSplit(cmd_result, "\n", false);
	std::vector<std::vector<std::string>> devices_list;
	for (size_t i = 0; i < dev_lines.size(); i++){		
		dev_lines[i] = pfacesUtils::strReplaceAll(dev_lines[i], "\t", "");
		dev_lines[i] = pfacesUtils::strReplaceAll(dev_lines[i], "[", "");

		std::string dev_id = "";
		std::string dev_type = "";
		std::string dev_name = "";
		try {
			std::vector<std::string> two_parts = pfacesUtils::strSplit(dev_lines[i], "]", false);
			std::string part_one = two_parts[0];
			std::string part_two = two_parts[1];

			dev_id = pfacesUtils::strSplit(part_one, ":", false)[0];
			dev_type = pfacesUtils::strSplit(part_one, ":", false)[1];
			dev_name = pfacesUtils::strSplit(part_two, ":", false)[1];

			dev_id = pfacesUtils::strReplaceAll(dev_id, " ", "");
			dev_type = pfacesUtils::strReplaceAll(dev_type, " ", "");
		}
		catch (...) {
			if (dev_id.empty())
				dev_id = "could not be resolved.";

			if (dev_type.empty())
				dev_type = "could not be resolved.";

			if (dev_name.empty())
				dev_name = "could not be resolved.";
		}

		std::vector<std::string> dev_info;
		dev_info.push_back(dev_id);
		dev_info.push_back(dev_type);
		dev_info.push_back(dev_name);
		devices_list.push_back(dev_info);
	}

	std::vector<std::string> table_head_keys = {
		PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_ID_JSON_KEY,
		PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_TYPE_JSON_KEY,
		PFACES_AGENT_USER_DICT_DEVICE_LIST_DEVICE_NAME_JSON_KEY
	};
	return pfacesAgentUtils::makeJSONTable(table_head_keys, devices_list);
}
std::string getProjectsList(std::string user_path) {
	std::vector<std::string> projects_dirs = pfacesAgentUtils::get_path_directories(user_path);
	std::string ret;
	for (size_t i = 0; i < projects_dirs.size(); i++){
		ret += projects_dirs[i];
		if (i != (projects_dirs.size() - 1))
			ret += ", ";
	}
	return 	ret;
}


bool 
pFacesAgentHelper::isLoginNew(const std::string& userLoginJSONstr) {
	// extract premissiton value
	std::string premission_value =
		pfacesAgentUtils::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION);

	// check it
	if (premission_value == std::string(PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_requested))
		return true;
	else
		return false;
}

bool 
pFacesAgentHelper::isLoginPermitted(const std::string& userLoginJSONstr) {
	
	std::string user_id =
		pfacesAgentUtils::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_ID);

	std::string user_key =
		pfacesAgentUtils::getJSONItemValue(userLoginJSONstr, PFACES_AGENT_LOGIN_DICT_USER_KEY);

	// todo: implement a login logic by connecting to a login server
	if (user_id == user_id && user_key == user_key)
		return true;
	else
		return false; /*not happening*/
}

std::string 
pFacesAgentHelper::setLoginInfo(const std::string& userLoginJSONstr, const std::string& loginUrl, const std::string& loginPort) {

	std::string ret = userLoginJSONstr;
	ret = pfacesAgentUtils::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_LOGIN_URL, loginUrl);
	ret = pfacesAgentUtils::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_LOGIN_PORT, loginPort);
	return ret;
}

std::string 
pFacesAgentHelper::setLoginDone(const std::string& userLoginJSONstr, bool success, const std::string& msg) {
	std::string ret = userLoginJSONstr;
	if(success)
		ret = pfacesAgentUtils::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION,
			PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_granted);
	else
		ret = pfacesAgentUtils::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_PERMISSION,
			PFACES_AGENT_LOGIN_DICT_USER_PERMISSION_VALUE_denied);
	
	ret = pfacesAgentUtils::updateJSONItemValue(ret, PFACES_AGENT_LOGIN_DICT_USER_INFO_MESSAGE, msg);
	
	return ret;
}

std::string pFacesAgentHelper::addToActivityLog(const std::string& oldActivityLog, const std::string& newLogStatus) {
	return oldActivityLog + std::string(",\"yy.dd.mm hh:ss\":") + newLogStatus;
}

std::vector<std::pair<std::string, std::string>>
pFacesAgentHelper::initializeUserDictionary(const std::string& user_id, const AgentConfigs& configs) {

	std::vector<std::pair<std::string, std::string>> keyValuePairs;

	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_USER_ID, user_id));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_INFO, std::string("pFacesAgent User Dictionary for user_id=") + user_id));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_AGENT_VERSION, "1.0"));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_PFACES_VERSION, getpFacesVersion()));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_AGENT_STATUS, PFACES_AGENT_USER_DICT_AGENT_STATUS_busy));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_PROJECT_LIST, getProjectsList(configs.user_data_directory + std::string(PATH_DELIMITER) + user_id + std::string(PATH_DELIMITER))));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_DEVICE_LIST, getDevicesListJSON(configs.device_mask)));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_JOBS_LIST, ""));

	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_UPLOAD, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_COMPILE, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN, ""));
	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_KILL, ""));

	keyValuePairs.push_back(std::make_pair(PFACES_AGENT_USER_DICT_ACTIVITY_LOG, addToActivityLog("","Dictionary created.")));

	return keyValuePairs;
}

std::string pFacesAgentHelper::commandRequestToString(pFacesAgentHelper::COMMAND_REQUESTS cr_type) {
	for (size_t i = 0; i < CR_Types.size(); i++){
		if (cr_type == CR_Types[i])
			return CRs_to_check[i];
	}	
	return "cr_none";
}


size_t pFacesAgentHelper::countPendingCommandRequest(pFacesAgentUserContext& userContext) {

	size_t cnt = 0;
	for (size_t i = 0; i < CRs_to_check.size(); i++){
		std::string cr_json = userContext.spUserDictionary->getKeyValue(CRs_to_check[i]);
		std::string cr_status = pfacesAgentUtils::getJSONItemValue(cr_json, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY);

		if (cr_status == PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_submitted)
			cnt++;
	}
	return cnt;
}

std::pair<pFacesAgentHelper::COMMAND_REQUESTS, bool> 
pFacesAgentHelper::processNextPendingCommandRequest(pFacesAgentUserContext& userContext) {


	for (size_t i = 0; i < CRs_to_check.size(); i++){
		std::string cr_json = userContext.spUserDictionary->getKeyValue(CRs_to_check[i]);
		std::string cr_status = pfacesAgentUtils::getJSONItemValue(cr_json, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY);
		if (cr_status == PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_submitted) {
			
			cr_json = pfacesAgentUtils::updateJSONItemValue(
				cr_json,
				PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY,
				PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_processing);
			userContext.spUserDictionary->setKeyValue(
				CRs_to_check[i],
				cr_json);

			try {
				// do the task
				CR_processors[i](userContext);

				cr_json = pfacesAgentUtils::updateJSONItemValue(
					cr_json,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_done);
				userContext.spUserDictionary->setKeyValue(
					CRs_to_check[i],
					cr_json);

				// clear any blob
				cr_json = pfacesAgentUtils::updateJSONItemValue(cr_json,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_BLOB_JSON_KEY, "");
				userContext.spUserDictionary->setKeyValue(
					CRs_to_check[i],
					cr_json);

				return std::make_pair(CR_Types[i], true);

			}
			catch (std::exception ex) {

				cr_json = pfacesAgentUtils::updateJSONItemValue(
					cr_json,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_JSON_KEY,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_STATUS_error);
				cr_json = pfacesAgentUtils::updateJSONItemValue(
					cr_json,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_MESSAGE_JSON_KEY,
					std::string("exception: ") + std::string(ex.what()));
				userContext.spUserDictionary->setKeyValue(
					CRs_to_check[i],
					cr_json);

				// clear any blob
				cr_json = pfacesAgentUtils::updateJSONItemValue(cr_json,
					PFACES_AGENT_USER_DICT_COMMAND_REQUEST_BLOB_JSON_KEY, "");
				userContext.spUserDictionary->setKeyValue(
					CRs_to_check[i],
					cr_json);

				return std::make_pair(CR_Types[i], false);
			}
		}
	}

	return std::make_pair(COMMAND_REQUESTS::NONE, true);
}



// CR Processors
void pFacesAgentHelper::processCRUpload(pFacesAgentUserContext& userContext) {

	// save the project file from the blob
	std::string cr_upload = userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_UPLOAD);

	// upload-specific stuff
	std::string proj_name = pfacesAgentUtils::getSubJSONItemValue(
		cr_upload, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_UPLOAD_PROJECT_name);

	std::string proj_blob64 = pfacesAgentUtils::getJSONItemValue(cr_upload, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_BLOB_JSON_KEY);
	//std::string proj_blob64 = pfacesAgentUtils::getSubJSONItemValue( cr_upload, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY, PFACES_AGENT_USER_DICT_PROJECT_UPLOAD_PROJECT_blob);

	if (proj_name.empty() || proj_blob64.empty()) {
		throw std::runtime_error("project name and project blob must not be empty");
	}

	// decode the blob
	std::vector<unsigned char> blob = Base64::decode(proj_blob64);

	// save as binary
	std::string user_id = userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_USER_ID);
	std::string user_dir = userContext.spAgentConfigs.user_data_directory + std::string(PATH_DELIMITER) + user_id;
	std::string file_path = user_dir + std::string(PATH_DELIMITER) + proj_name + std::string(".zip");

	pfacesAgentUtils::make_directory(user_dir);
	auto file = std::fstream(file_path, std::ios::out | std::ios::binary);
	file.write((const char*)blob.data(), blob.size());
	file.close();

	// decompress
	std::string project_path = userContext.spAgentConfigs.user_data_directory + std::string(PATH_DELIMITER) + user_id + std::string(PATH_DELIMITER) + proj_name + std::string(PATH_DELIMITER);
	pfacesAgentUtils::make_directory(project_path);
	Zipper::uncompressFile(file_path, project_path);

	// update projects list
	std::string projects_list = userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_PROJECT_LIST);
	std::vector<std::string> projects = pfacesUtils::sStr2Vector<std::string>(projects_list);
	if (!pfacesUtils::isVectorElement(projects, proj_name)) {
		projects.push_back(proj_name);
	}
	projects_list = "";
	for (size_t i = 0; i < projects.size(); i++) {
		if (projects[i].empty() || projects[i] == std::string(""))
			continue;
		projects_list += projects[i];
		if (i != (projects.size() - 1))
			projects_list += ", ";
	}
	userContext.spUserDictionary->setKeyValue(PFACES_AGENT_USER_DICT_PROJECT_LIST, projects_list);
}
void pFacesAgentHelper::processCRCompile(pFacesAgentUserContext& userContext) {

	std::string cr_compile =
		userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_COMPILE);

	std::string proj_name = pfacesAgentUtils::getSubJSONItemValue(
		cr_compile, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_COMPILE_PROJECT_name);

	if (proj_name.empty())
		throw std::runtime_error("project name must not be empty");

	// make the command
	std::string user_id = userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_USER_ID);
	std::string project_path =
		userContext.spAgentConfigs.user_data_directory + std::string(PATH_DELIMITER) + user_id +
		std::string(PATH_DELIMITER) + proj_name + std::string(PATH_DELIMITER);

	std::string cmd = userContext.spAgentConfigs.build_command;
	cmd = pfacesUtils::strReplaceAll(cmd, "%PROJECT_PATH%", project_path);
	cmd = pfacesUtils::strReplaceAll(cmd, "%PROJECT_NAME%", proj_name);
	cmd = pfacesUtils::strReplaceAll(cmd, "'", "\"");

	size_t jobId = userContext.spJobManager->launchJob(cmd);
	if (jobId == 0)
		throw std::runtime_error("Failed to lauch the build job.");
}
void pFacesAgentHelper::processCRRun(pFacesAgentUserContext& userContext) {
	std::string cr_run = 
		userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN);

	// run-specific stuff
	std::string proj_name = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_PROJECT_name);
	std::string dev_id = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_DEVICE_id);
	std::string kernel_name = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_KERNEL_name);
	std::string kernel_dir = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_KERNEL_dir);
	std::string config_path = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_CONFIG_path);
	std::string config_run_extras = pfacesAgentUtils::getSubJSONItemValue(
		cr_run, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_RUN_extras);

	if(proj_name.empty() || dev_id.empty() || kernel_name.empty() || kernel_dir.empty())
		throw std::runtime_error("project name, device id, and kernel name/dir must not be empty");

	if (kernel_dir == std::string("."))
		kernel_dir = std::string("kernel-pack");

	// make the command
	std::string user_id = userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_USER_ID);
	std::string project_path = 
		userContext.spAgentConfigs.user_data_directory + std::string(PATH_DELIMITER) + user_id +
		std::string(PATH_DELIMITER) + proj_name + std::string(PATH_DELIMITER);

	std::string cmd =
		std::string("pfaces -") + userContext.spAgentConfigs.device_mask +
		std::string(" -d ") + dev_id +
		std::string(" -p ") +
		std::string(" -k ") + kernel_name + std::string("@") + project_path + kernel_dir +
		
		(
		(config_path.empty())?
			(std::string("")):
			(std::string(" -cfg ") + project_path + std::string(PATH_DELIMITER) + config_path)
		) 
		+ 
		(
			config_run_extras.empty()?
			std::string(""):
			(std::string(" ") + config_run_extras)
		);

	// launch it
	size_t jobId = userContext.spJobManager->launchJob(cmd);
	if(jobId == 0)
		throw std::runtime_error("Failed to lauch the pFaces run job.");
}
void pFacesAgentHelper::processCRKill(pFacesAgentUserContext& userContext) {

	std::string cr_kill =
		userContext.spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_COMMAND_REQUEST_RUN);

	// kill-specific stuff
	std::string job_id = pfacesAgentUtils::getSubJSONItemValue(
		cr_kill, PFACES_AGENT_USER_DICT_COMMAND_REQUEST_OPTION_JSON_KEY,
		PFACES_AGENT_USER_DICT_PROJECT_KILL_JOB_id);

	try {
		size_t id = atoll(job_id.c_str());
		userContext.spJobManager->killJob(id);
	}
	catch (...) {
		throw std::runtime_error("Failed to kill the pFaces job.");
	}

}