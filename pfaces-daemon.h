#ifndef PFACES_DAEMON_H
#define PFACES_DAEMON_H

#include "pfaces-agent.h"

class pFacesDaemon {
	bool install();
	bool uninstall();
	bool run(const std::vector<AgentConfigs>& per_agent_config);
public:
	pFacesDaemon() = default;
	~pFacesDaemon() = default;
	bool launch(LaunchModes mode, const std::vector<AgentConfigs>& per_agent_config);
};

#endif // ! PFACES_DAEMON_H