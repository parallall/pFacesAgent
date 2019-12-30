#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "WindowsServiceInstaller.h"
#include "pfaces-service.h"
#include "Logger.h"
#include "WindowsThreadPool.h"


// Internal name of the service 
#define SERVICE_NAME             "pFacesAgent" 
// Displayed name of the service 
#define SERVICE_DISPLAY_NAME     "pFacesAgent" 
// Description of the service 
#define SERVICE_DESCRIPYION     "Creates one or more of RESTful agents to launch jobs for pFaces." 
// Service start options. 
#define SERVICE_START_TYPE       SERVICE_DEMAND_START 
// List of service dependencies - "dep1\0dep2\0\0" 
#define SERVICE_DEPENDENCIES     "" 
// The name of the account under which the service should run 
#define SERVICE_ACCOUNT          NULL	//"NT AUTHORITY\\LocalService" 
// The password to the service account name 
#define SERVICE_PASSWORD         NULL

// Some options of the service
#define SERVICE_CAN_STOP TRUE
#define SERVICE_CAN_SHUTDOWN TRUE 
#define SERVICE_CAN_CONTINUE FALSE

// sloop time between ragent calls
#define AGENT_SLEEP_PERIOD_MS 10



pFacesServiceManager::pFacesServiceManager(const std::vector<AgentConfigs>& per_agent_config):
CServiceBase((LPSTR)SERVICE_NAME, 
	SERVICE_CAN_STOP, SERVICE_CAN_SHUTDOWN, SERVICE_CAN_CONTINUE){

	m_per_agent_config = per_agent_config;

	m_fStopping = FALSE;

	// Create a manual-reset event that is not signaled at first to indicate 
	// the stopped signal of the service.
	m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hStoppedEvent == NULL){
		throw GetLastError();
	}
}

pFacesServiceManager::~pFacesServiceManager() {
	if (m_hStoppedEvent){
		CloseHandle(m_hStoppedEvent);
		m_hStoppedEvent = NULL;
	}
}

bool pFacesServiceManager::launch(LaunchModes mode) {
	if (mode == LaunchModes::RUN) {
		if (!CServiceBase::Run(*this)) {
			Logger::log("pfaces-service/lauch", 
				std::string("Running windows servecie failed with error code:") +
				std::to_string(GetLastError())
			);
			return false;
		}
		return true;
	}
	else if (mode == LaunchModes::INSTALL) {
		Logger::log("pfaces-service/lauch", "Started installing the service.");
		if (!install()) {
			Logger::log("pfaces-service/lauch", "Failed to install the service.");
			return false;
		}
		Logger::log("pfaces-service/lauch", "Done installing the service.");
		return true;
	} else if (mode == LaunchModes::UNINSTALL) {
		Logger::log("pfaces-service/lauch", "Started uninstalling the service.");
		if (!uninstall()) {
			Logger::log("pfaces-service/lauch", "Failed to uninstall the service.");
			return false;
		}
		Logger::log("pfaces-service/lauch", "Done uninstalling the service.");
		return true;
	}
	else {
		Logger::log("pfaces-service/lauch", "Unknown launch mode!");
		return false;
	}
}

bool pFacesServiceManager::install() {
	std::stringstream ss;
	DWORD err =
		InstallService( SERVICE_NAME, SERVICE_DISPLAY_NAME,
			SERVICE_START_TYPE, SERVICE_DEPENDENCIES,
			SERVICE_ACCOUNT, SERVICE_PASSWORD, ss);

	if (err == 0) {
		Logger::log("pfaces-service/install", 
			std::string("Installing the service succeeded with the log: \n") + ss.str());

		ss.clear();
		err = ChangeServiceDescription(SERVICE_NAME, SERVICE_DESCRIPYION, ss);
		if (err != 0) {
			Logger::log("pfaces-service/install",
				std::string("Changing the service description failed with the log: \n") + ss.str());
		}
		

		return true;
	}
	else {
		Logger::log("pfaces-service/install", 
			(std::string("Installing the service failed with the log:\n")+ ss.str()));
		return false;
	}
}
bool pFacesServiceManager::uninstall() {
	std::stringstream ss;
	DWORD err =
		UninstallService(SERVICE_NAME, ss);

	if (err == 0) {
		Logger::log("pfaces-service/uninstall",
			std::string("Uninstalling the service succeeded with the log: \n") + ss.str());
		return true;
	}
	else {
		Logger::log("pfaces-service/uninstall", 
			(std::string("Uninstalling the service failed with the log:\n") + ss.str()));
		return false;
	}
}


void pFacesServiceManager::OnStart(DWORD dwArgc, LPSTR* lpszArgv)
{
	// Queue the main service function for execution in a worker thread.
	Logger::log("pfaces-service/OnStart", "Launching the agent thread.");
	WindowsThreadPool::QueueUserWorkItem(&pFacesServiceManager::run, this);
}

void pFacesServiceManager::OnStop()
{
	// Log a service stop message to the Application log.
	Logger::log("pfaces-service/OnStop", "Stopping the agent thread.");

	// Indicate that the service is stopping and wait for the finish of the 
	// main service function (ServiceWorkerThread).
	m_fStopping = TRUE;
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0){
		throw GetLastError();
	}
}



void pFacesServiceManager::run(){

	size_t num_agents = m_per_agent_config.size();

	// creating the agents
	Logger::log("pfaces-service/run", 
		std::string("Creating ") + std::to_string(num_agents) + std::string(" pfaces RESTFul agents.")
	);
	for (size_t i = 0; i < num_agents; i++) {
		m_agents.push_back(pFacesAgent(m_per_agent_config[i]));
	}

	// run the agents
	for (size_t i = 0; i < num_agents; i++)
		m_agents[i].run();

	// Periodically check if the service is stopping.
	while (!m_fStopping){
		::Sleep(AGENT_SLEEP_PERIOD_MS);  // Simulate some lengthy operations.
	}

	// stop the agents
	Logger::log("pfaces-service/run", "Recieved a stop signal. Killing the agents ...");
	for (size_t i = 0; i < num_agents; i++)
		m_agents[i].kill();

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent);
}
	
