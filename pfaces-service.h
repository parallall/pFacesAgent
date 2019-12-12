#ifndef PFACES_SERVICE_H
#define PFACES_SERVICE_H

#include "pfaces-agent.h"
#include "WindowsServiceBase.h"

class pFacesServiceManager : public CServiceBase {
	std::vector<AgentConfigs> m_per_agent_config;

	BOOL m_fStopping;
	HANDLE m_hStoppedEvent;

	bool install();
	bool uninstall();

	void run();

	// list of agents
	std::vector<pFacesAgent> m_agents;


	// Maybe later: a shared memory between agents in form of in/out message boxes
	// or we may utilize MPI

public:
	pFacesServiceManager(const std::vector<AgentConfigs>& per_agent_config);
	~pFacesServiceManager();
	bool launch(LaunchModes mode);

protected:
	virtual void OnStart(DWORD dwArgc, LPSTR* pszArgv);
	virtual void OnStop();
};


#endif // ! PFACES_SERVICE_H