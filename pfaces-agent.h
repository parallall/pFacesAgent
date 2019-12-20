#ifndef PFACES_AGENT_H
#define PFACES_AGENT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "pfacesRemoteInterface.h"
#include "pfaces-agent-helper.h"

enum LaunchModes {
	RUN,
	INSTALL,
	UNINSTALL,
};

// pFacesAgent class
// =================
// pFacesAgent is responsible for launching pFaces according to different scenarios
// the service/daemon will firs tinitiate at least one pFacesAgent object
// each pFacesAgent object should be associated to one unique port which it will listen to.
// the run function of the pFacesAgent will be continously called by the service/daemon with some time gab in betwwen
// the servant should not mae "run" blocking.
// The agent assumes its unique possition of the resources it is assigned to using the device_mask/device_ids_list.
// The agent is responsible for scheduling betweeen different requests competing on its resources.
// The agent may help synchronize data with other agents.
class pFacesAgent {
public:
	AgentConfigs m_configs;

	// the dictionary severs
	std::shared_ptr<pfacesRESTfullDictionaryServer> LoginDictionaryServer;	
	std::vector<std::shared_ptr<pfacesRESTfullDictionaryServer>> userDictionaryServers;

	// active serving threads
	std::shared_ptr<std::thread> loginManagerThread;
	std::vector<std::shared_ptr<std::thread>> userManagerThreads;

	// kill signal for all threads
	bool kill_signal = false;

	// last assigned port 
	size_t last_assigned_port;

	pFacesAgent(const AgentConfigs& configs);
	~pFacesAgent() = default; 

	// run will be called often from the service/daemon and should not be blocking
	// it should sense the port and read any requests, then, launch pFaces with it.
	// upon launchin a new pFaces job, it should store the lauch settings and track pFaaces.
	// It may decide to delay another launch of pFAces if oneis already running in the devices
	// assicliated
	int run();
	int kill();

	bool initializeLoginDictionary();
};

#endif // ! PFACES_AGENT_H

