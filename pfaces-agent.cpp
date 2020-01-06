#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
using namespace std::chrono_literals;

#include "pfaces-agent.h"
#include "pfaces-job-manager.h"
#include "Logger.h"


#define PFACES_AGENT_HOST "*"
#define PFACES_AGENT_NAME_LOGIN "pFacesAgentLogin"
#define PFACES_AGENT_NAME_CLEINT "pFacesAgent-U"
#define THREAD_SLEEP_TIME 100ms


// serving thread functions
void userManager(std::shared_ptr<pFacesAgent> spAgent, size_t userIndexInDictionary) {
	
	// getting the dictionary
	std::shared_ptr<pfacesRESTfullDictionaryServer> spUserDictionary = spAgent->userDictionaryServers[userIndexInDictionary];
	std::string user_id = spUserDictionary->getKeyValue(PFACES_AGENT_USER_DICT_USER_ID);

	// creating a job manager
	std::shared_ptr<pfacesJobManager> spJobManager = std::make_shared<pfacesJobManager>();

	// create the user context
	pFacesAgentUserContext userContext;
	userContext.spJobManager = spJobManager;
	userContext.spUserDictionary = spUserDictionary;
	userContext.spAgentConfigs = spAgent->m_configs;

	
	// declare ready status
	spUserDictionary->setKeyValue(PFACES_AGENT_USER_DICT_AGENT_STATUS, PFACES_AGENT_USER_DICT_AGENT_STATUS_ready);

	// preocess requests
	bool the_kill_signal = false;
	while (!the_kill_signal) {

		// process any submitted CR
		size_t cnt_pending_cr = pFacesAgentHelper::countPendingCommandRequest(userContext);
		if(cnt_pending_cr != 0){
			// set to busy
			spUserDictionary->setKeyValue(PFACES_AGENT_USER_DICT_AGENT_STATUS, PFACES_AGENT_USER_DICT_AGENT_STATUS_busy);

			std::pair<pFacesAgentHelper::COMMAND_REQUESTS, bool> cr_process_result =
			pFacesAgentHelper::processNextPendingCommandRequest(userContext);
			if (cr_process_result.first != pFacesAgentHelper::COMMAND_REQUESTS::NONE) {
				Logger::log("pFacesAgent/userManager", std::string("For user_id = ") + user_id +
					std::string(" we served a command-request of type ") + pFacesAgentHelper::commandRequestToString(cr_process_result.first) +
					std::string(" with result=") + 
					std::string(cr_process_result.second?"success":"failure")
				);
			}

			// back to ready
			spUserDictionary->setKeyValue(PFACES_AGENT_USER_DICT_AGENT_STATUS, PFACES_AGENT_USER_DICT_AGENT_STATUS_ready);
		}

		// update jobs table
		std::string jobsTableJson = spJobManager->getJobsTableJSON(
			PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_ID_JSON_KEY,
			PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_ID_JSON_CMD,
			PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_STATUS_JSON_KEY,
			PFACES_AGENT_USER_DICT_JOBS_LIST_JOB_DETAILS_JSON_KEY
		);
		spUserDictionary->setKeyValue(
			PFACES_AGENT_USER_DICT_JOBS_LIST,
			jobsTableJson);


		// check other commands 
		// TODO

		// sleep for a while !
		std::this_thread::sleep_for(THREAD_SLEEP_TIME);



		std::lock_guard<std::mutex> lock(*(spAgent->sp_kill_signal_mutex));
		the_kill_signal = spAgent->kill_signal;
	}
}

size_t addNewUser(
	std::string user_id,
	std::shared_ptr<pFacesAgent> spAgent,
	size_t newUserPort) {

	// Add item to userDictionaryServers and fill initial values in this item
	std::shared_ptr<pfacesRESTfullDictionaryServer> userDictionary = std::make_shared<pfacesRESTfullDictionaryServer>();
	auto keyValuePairs = pFacesAgentHelper::initializeUserDictionary(user_id, spAgent->m_configs);
	std::vector<std::string> keysOnly;
	for (size_t i = 0; i < keyValuePairs.size(); i++)
		keysOnly.push_back(keyValuePairs[i].first);	
	userDictionary->setup(PFACES_AGENT_HOST, std::to_string(newUserPort), keysOnly, std::string(PFACES_AGENT_NAME_CLEINT) + user_id);
	for (size_t i = 0; i < keyValuePairs.size(); i++)
		userDictionary->setKeyValue(keyValuePairs[i].first, keyValuePairs[i].second);
	spAgent->userDictionaryServers.push_back(userDictionary);
	size_t userIndexInDictionary = spAgent->userDictionaryServers.size() - 1;
	
	// Create a thread in (userManagerThreads) to call (userManager) with this item
	std::shared_ptr<std::thread> userManagerThread = 
		std::make_shared<std::thread>(userManager, spAgent, userIndexInDictionary);
	spAgent->userManagerThreads.push_back(userManagerThread);

	return userIndexInDictionary;
}

void loginManager(std::shared_ptr<pFacesAgent> spAgent) {
	bool the_kill_signal = false;
	while (!the_kill_signal) {

		const std::vector<std::string>& allUserIds = spAgent->LoginDictionaryServer->getAllKeys();

		// iterate over all login items
		for (size_t i = 0; i < allUserIds.size(); i++){
			std::string user_id = allUserIds[i];
			std::string userLoginJSONstr = spAgent->LoginDictionaryServer->getKeyValue(user_id);

			if (pFacesAgentHelper::isLoginNew(userLoginJSONstr)) {

				Logger::log("pFacesAgent/loginManager", std::string("Login manger recieved a new login request from user_id = ") + user_id);
				bool success = false;
				size_t newUserPort = spAgent->last_assigned_port + 1;
				std::string msg = "";
				try {
					size_t userIndexInDictionary = addNewUser(user_id, spAgent, newUserPort);
					Logger::log("pFacesAgent/loginManager", std::string("Login manger created a new user for user_id = ") + user_id);

					std::string loginUrl = spAgent->userDictionaryServers[userIndexInDictionary]->getUrl();
					std::string loginPort = std::to_string(newUserPort);

					Logger::log("pFacesAgent/loginManager", std::string("The user can use the following REST node:") + loginUrl);

					userLoginJSONstr = pFacesAgentHelper::setLoginInfo(userLoginJSONstr, loginUrl, loginPort);
					success = true;
				}
				catch(...){
					Logger::log("pFacesAgent/loginManager", std::string("Login manger failed to create a new user for user_id = ") + user_id);
				}		

				
				userLoginJSONstr = pFacesAgentHelper::setLoginDone(userLoginJSONstr, success, msg);
				spAgent->LoginDictionaryServer->setKeyValue(user_id, userLoginJSONstr);
			}
		}

		// sleep for a while !
		std::this_thread::sleep_for(THREAD_SLEEP_TIME);

		// update the kill signal
		std::lock_guard<std::mutex> lock(*(spAgent->sp_kill_signal_mutex));
		the_kill_signal = spAgent->kill_signal;
	}
	Logger::log("pFacesAgent/loginManager", std::string("Login thread is done."));
}




// private members
bool pFacesAgent::initializeLoginDictionary() {
	
	// the initial keys
	std::vector<std::string> loginDictionaryKeys;
	loginDictionaryKeys.push_back("status");

	// setting up the dictionary server
	LoginDictionaryServer = std::make_shared<pfacesRESTfullDictionaryServer>();
	std::string agentId = std::string(PFACES_AGENT_NAME_LOGIN) + m_configs.id;
	LoginDictionaryServer->setup(
		PFACES_AGENT_HOST, std::to_string(m_configs.listen_port),
		loginDictionaryKeys, agentId);


	LoginDictionaryServer->setKeyValue("status", "ready");

	last_assigned_port = m_configs.listen_port;

	Logger::log("pFacesAgent/initializeLoginDictionary", "Login dictionary REST server is running and users now can set/get values from it using the URL:");
	Logger::log("pFacesAgent/initializeLoginDictionary", LoginDictionaryServer->getUrl().c_str());

	return true;
}


// public members
pFacesAgent::pFacesAgent(const AgentConfigs& configs) {
	m_configs = configs;
	sp_kill_signal_mutex = std::make_shared<std::mutex>();
	spThis = std::shared_ptr<pFacesAgent>(this);
}

int pFacesAgent::run() {
	// initialize and run login manager
	if (!initializeLoginDictionary()) {
		Logger::log("pFacesAgent/run", "Failed to initialize the login dictionary !");
		return -1;
	}

	// run the login manager thread
	try {
		loginManagerThread = std::make_shared<std::thread>(loginManager, spThis);
		Logger::log("pFacesAgent/run", "A thread is created to handle user login requests. pFaces-Agent is now running !");
	}
	catch (...) {
		Logger::log("pFacesAgent/run", "Failed to create a thread for user login requests.");
		return -1;
	}

	return 0;
}

int pFacesAgent::kill() {
	try {
		{
			std::lock_guard<std::mutex> lock(*(sp_kill_signal_mutex));
			kill_signal = true;
		}
		Logger::log("pFacesAgent/kill", "kill signal raised. Joining the agent login thread ... ");

		if(loginManagerThread->joinable())
			loginManagerThread->join();

		Logger::log("pFacesAgent/kill", "Joining the agent user threads ... ");
		for (size_t i = 0; i < userManagerThreads.size(); i++){
			if (userManagerThreads[i]->joinable())
				userManagerThreads[i]->join();
		}

		Logger::log("pFacesAgent/kill", "Finished killing all active threads.");
		return 0;
	}
	catch (std::exception ex) {
		Logger::log("pFacesAgent/kill", 
			std::string("Failed to kill all active threads: ") + ex.what()
		);
		return -1;
	}	
}