#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
using namespace std::chrono_literals;

#include "pfaces-agent.h"
#include "pfaces-agent-helper.h"
#include "Logger.h"


#define PFACES_AGENT_HOST "*"
#define PFACES_AGENT_NAME_LOGIN "pFacesAgentLogin"
#define PFACES_AGENT_NAME_CLEINT "pFacesAgent-U"
#define THREAD_SLEEP_TIME 100ms


// serving thread functions
void userManager(pFacesAgent* pAgent, size_t userIndexInDictionary) {
	while (!pAgent->kill_signal) {


		// sleep for a while !
		std::this_thread::sleep_for(THREAD_SLEEP_TIME);
	}
}

size_t addNewUser(
	std::string user_id,
	pFacesAgent* pAgent,
	size_t newUserPort) {

	// Add item to userDictionaryServers and fill initial values in this item
	std::shared_ptr<pfacesRESTfullDictionaryServer> userDictionary = std::make_shared<pfacesRESTfullDictionaryServer>();
	auto keyValuePairs = pFacesAgentHelper::initializeUserDictionary(user_id);
	std::vector<std::string> keysOnly;
	for (size_t i = 0; i < keyValuePairs.size(); i++)
		keysOnly.push_back(keyValuePairs[i].first);	
	userDictionary->setup(PFACES_AGENT_HOST, std::to_string(newUserPort), keysOnly, std::string(PFACES_AGENT_NAME_CLEINT) + user_id);
	for (size_t i = 0; i < keyValuePairs.size(); i++)
		userDictionary->setKeyValue(keyValuePairs[i].first, keyValuePairs[i].second);
	pAgent->userDictionaryServers.push_back(userDictionary);
	size_t userIndexInDictionary = pAgent->userDictionaryServers.size() - 1;
	
	// Create a thread in (userManagerThreads) to call (userManager) with this item
	std::shared_ptr<std::thread> userManagerThread = std::make_shared<std::thread>(userManager, pAgent, userIndexInDictionary);
	pAgent->userManagerThreads.push_back(userManagerThread);

	return userIndexInDictionary;
}

void loginManager(pFacesAgent* pAgent) {
	while (!pAgent->kill_signal) {

		const std::vector<std::string>& allUserIds = pAgent->LoginDictionaryServer->getAllKeys();

		// iterate over all login items
		for (size_t i = 0; i < allUserIds.size(); i++){
			std::string user_id = allUserIds[i];
			std::string userLoginJSONstr = pAgent->LoginDictionaryServer->getKeyValue(user_id);

			if (pFacesAgentHelper::isLoginNew(userLoginJSONstr)) {

				Logger::log("pFacesAgent/loginManager", std::string("Login manger recieved a new login request from user_id = ") + user_id);
				bool success = false;
				size_t newUserPort = pAgent->last_assigned_port + 1;
				std::string msg = "";
				try {
					size_t userIndexInDictionary = addNewUser(user_id, pAgent, newUserPort);
					Logger::log("pFacesAgent/loginManager", std::string("Login manger created a new user for user_id = ") + user_id);

					std::string loginUrl = pAgent->userDictionaryServers[userIndexInDictionary]->getUrl();
					std::string loginPort = std::to_string(newUserPort);
					userLoginJSONstr = pFacesAgentHelper::setLoginInfo(userLoginJSONstr, loginUrl, loginPort);
					success = true;
				}
				catch(...){
					Logger::log("pFacesAgent/loginManager", std::string("Login manger failed to create a new user for user_id = ") + user_id);
				}		

				
				userLoginJSONstr = pFacesAgentHelper::setLoginDone(userLoginJSONstr, success, msg);
				pAgent->LoginDictionaryServer->setKeyValue(user_id, userLoginJSONstr);
			}
		}

		// sleep for a while !
		std::this_thread::sleep_for(THREAD_SLEEP_TIME);
	}
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
	Logger::log("pFacesAgent/initializeLoginDictionary", LoginDictionaryServer->getUrl());

	return true;
}


// public members
pFacesAgent::pFacesAgent(const AgentConfigs& configs) {
	m_configs = configs;
}

int pFacesAgent::run() {
	// initialize and run login manager
	if (!initializeLoginDictionary()) {
		Logger::log("pFacesAgent/run", "Failed to initialize the login dictionary !");
		return -1;
	}

	// run the login manager thread
	try {
		loginManagerThread = std::make_shared<std::thread>(loginManager, this);
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
		kill_signal = true;
		loginManagerThread->join();
		for (size_t i = 0; i < userManagerThreads.size(); i++){
			userManagerThreads[i]->join();
		}
		Logger::log("pFacesAgent/kill", "Finished killing all active threads.");
		return 0;
	}
	catch (...) {
		Logger::log("pFacesAgent/kill", "Failed to kill all active threads.");
		return -1;
	}	
}