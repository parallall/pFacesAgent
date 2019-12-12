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
#define SERVICE_DISPLAY_NAME     "Initializes one or more of RESTful agents each on a separate port. They respond to commands for starting pFaces." 
// Service start options. 
#define SERVICE_START_TYPE       SERVICE_DEMAND_START 
// List of service dependencies - "dep1\0dep2\0\0" 
#define SERVICE_DEPENDENCIES     "" 
// The name of the account under which the service should run 
#define SERVICE_ACCOUNT          "NT AUTHORITY\\LocalService" 
// The password to the service account name 
#define SERVICE_PASSWORD         NULL

// Some options of the service
#define SERVICE_CAN_STOP TRUE
#define SERVICE_CAN_SHUTDOWN TRUE 
#define SERVICE_CAN_CONTINUE FALSE

// sloop time between ragent calls
#define AGENT_SLEEP_PERIOD_MS 100



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
	if (mode == LaunchModes::INSTALL) {
		Logger::log("pfaces-service/lauch", "Started installing the service.");
		return install();
	} else if (mode == LaunchModes::UNINSTALL) {
		Logger::log("pfaces-service/lauch", "Started uninstalling the service.");
		return uninstall();
	}
	else {
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
		Logger::log("pfaces-service/install", "Installing the service succeeded.");
		return true;
	}
	else {
		Logger::log("pfaces-service/install", (std::string("Installing the service failed with the log:\n")+ ss.str()));
		return false;
	}
}
bool pFacesServiceManager::uninstall() {
	std::stringstream ss;
	DWORD err =
		UninstallService(SERVICE_NAME, ss);

	if (err == 0) {
		Logger::log("pfaces-service/uninstall", "Unistalling the service succeeded.");
		return true;
	}
	else {
		Logger::log("pfaces-service/uninstall", (std::string("Uninstalling the service failed with the log:\n") + ss.str()));
		return false;
	}
}


void pFacesServiceManager::OnStart(DWORD dwArgc, LPSTR* lpszArgv)
{
	// Log a service start message to the Application log.
	Logger::log("pfaces-service/OnStart", "Launching the agent thread.");

	// Queue the main service function for execution in a worker thread.
	WindowsThreadPool::QueueUserWorkItem(&pFacesServiceManager::run, this);
}

void pFacesServiceManager::OnStop()
{
	// Log a service stop message to the Application log.
	Logger::log("pfaces-service/OnStop", "Launching the agent thread.");

	// Indicate that the service is stopping and wait for the finish of the 
	// main service function (ServiceWorkerThread).
	m_fStopping = TRUE;
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0){
		throw GetLastError();
	}
}



void pFacesServiceManager::run(){

	// creating the agents
	for (size_t i = 0; i < m_per_agent_config.size(); i++) {
		m_agents.push_back(pFacesAgent(m_per_agent_config[i]));
	}

	// Periodically check if the service is stopping.
	while (!m_fStopping)
	{
		// Perform main service function here...
		for (size_t i = 0; i < m_agents.size(); i++)
			m_agents[i].run();

		::Sleep(AGENT_SLEEP_PERIOD_MS);  // Simulate some lengthy operations.
	}

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent);
}
	
