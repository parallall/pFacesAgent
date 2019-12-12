/*NOTE: PFACES_BUILD_FOR_INTERFACES is defined in the project ! */
#include "pfaces-agent.h"
#include "Logger.h"
#include "pfacesConfiguration.h"
#include "pfacesIO.h"
#include <csignal>
#include <chrono>
#include <thread>

// OS-specific includes
#ifdef _MSC_VER	// VS on Windows ?
#include "pfaces-service.h"
#define pFacesAgentManager pFacesServiceManager
#else
#include "pfaces-daemon.h"
#define pFacesAgentManager pFacesDaemonManager
#endif // _MSC_VER

#define AGENT_MAIN "pfaces-agent\\main"

#define TEST_MODE
#ifdef TEST_MODE
std::shared_ptr<pFacesAgent> spSingleAgent;
void exitSignalHandler(int signum) {
	Logger::log("TEST_MODE/main", "Got CTRL-C signal. Exiting .... ");
	spSingleAgent->kill();
	exit(signum);
}
int main(int argc, char* argv[]) {
	LaunchModes launch_event;
	launch_event = LaunchModes::RUN;
	AgentConfigs agent_config;
	agent_config.id = "agent_0";
	agent_config.device_mask = "CG";
	agent_config.listen_port = 8000;

	spSingleAgent = std::make_shared<pFacesAgent>(agent_config);
	Logger::log("TEST_MODE/main", "launching a single agent !");
	int ret = spSingleAgent->run();
	if(ret == 0)
		Logger::log("TEST_MODE/main", "The single agent is launched successfully !");
	else
		Logger::log("TEST_MODE/main", "The single agent failed to launched !");

	Logger::log("TEST_MODE/main", "waiting for CTRL-C signal ... ");
	signal(SIGINT, exitSignalHandler);	
	while (true)std::this_thread::sleep_for(std::chrono::milliseconds(100));;
}
#else
int main(int argc, char* argv[]) {

	// Args
	std::string cmd = std::string(argv[1]);
	std::string config_file = std::string(argv[2]);
	
	// check the supplied arguments 
	if (argc != 3) {
		Logger::log(AGENT_MAIN, "Invalid number of arguments. pfaces-agent.exe must recieve the command and config file. Exiting ... !");
		return -1;
	}

	// The launch mode
	LaunchModes launch_event;
	if (cmd == std::string("-install")) {
		launch_event = LaunchModes::INSTALL;
		Logger::log(AGENT_MAIN, "Launch event set to install.");
	}
	else if (cmd == std::string("-uninstall")) {
		launch_event = LaunchModes::UNINSTALL;
		Logger::log(AGENT_MAIN, "Launch event set to uninstall.");
	}
	else if (cmd == std::string("-run")) {
		launch_event = LaunchModes::RUN;
		Logger::log(AGENT_MAIN, "Launch event set to run.");
	}
	else {
		Logger::log(AGENT_MAIN, "Invalid launch command. pfaces-agent.exe must recieve one of the following launch commands: -install, -uninstall, or -run. Exiting ... !");
		return -1;
	}

	// launch configurations
	if (!pfacesFileIO::isFileExist(config_file)) {
		Logger::log(AGENT_MAIN, "Configuration file not found. Existing ... !");
		return -1;
	}
	Logger::log(AGENT_MAIN, "reading the agents configs.");
	pfacesConfigurationReader config_reader = pfacesConfigurationReader(config_file, "");
	int num_agents = config_reader.readConfigValueInt("num_agents");
	if(num_agents < 1) {
		Logger::log(AGENT_MAIN, "Configuration file error: num_agents must be >= 1. Existing ... !");
		return -1;
	}
	std::vector<AgentConfigs> configs(num_agents);
	for (size_t i = 0; i < num_agents; i++) {
		std::string agent_config_path = std::string("agent") + std::to_string(i) + std::string(".");

		configs[i].id = config_reader.readConfigValueString(agent_config_path + std::string("id"));
		configs[i].device_mask = config_reader.readConfigValueString(agent_config_path + std::string("device_mask"));
		configs[i].listen_port = config_reader.readConfigValueInt(agent_config_path + std::string("listen_port"));
	}

	// launching
	Logger::log(AGENT_MAIN, "Creating the service/daemon and calling its launch method.");
	pFacesAgentManager agentManager(configs);
	if (launch_event == LaunchModes::RUN) {
		if (!CServiceBase::Run(agentManager)){
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
		}
	}
	else {
		agentManager.launch(launch_event);
	}
	
	
	// Maybe later:
	// here we can report to a centeral server that some agents are installed, uninstalled, ran or shut-down

	return 0;
}
#endif