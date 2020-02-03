/*NOTE: PFACES_BUILD_FOR_INTERFACES is defined in the project ! */
#include "pfaces-agent.h"
#include "pfaces-agent-utils.h"
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

#define AGENT_MAIN "pfaces-agent/main"


#define TEST_MODE
#ifdef TEST_MODE
std::shared_ptr<pFacesAgent> spSingleAgent;
void exitSignalHandler(int signum) {
	Logger::log("TEST_MODE/main", "Got CTRL-C signal. Exiting .... ");
	spSingleAgent->kill();
	exit(0);
}
int main(int argc, char* argv[]) {
	LaunchModes launch_event;
	launch_event = LaunchModes::RUN;
	AgentConfigs agent_config;
	agent_config.id = "agent_0";
	agent_config.device_mask = "CG";
	agent_config.listen_port = 8000;
	agent_config.user_data_directory = "C:\\pFacesAgent0_data\\";
	agent_config.build_command = "'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe' %PROJECT_PATH%\\%PROJECT_NAME%.sln /property:Configuration=Release /property:Platform=x64";

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
std::vector<AgentConfigs> fetchConfigs(std::string config_file) {
	pfacesConfigurationReader config_reader = pfacesConfigurationReader(config_file, "");
	try {
		config_reader.parse(nullptr, nullptr);
	} catch (std::exception ex) {
		Logger::log("fetchConfigs",
			std::string("Configuration parse error: ") +
			std::string(ex.what())
		);
		return {};
	}

	int num_agents = config_reader.readConfigValueInt("num_agents");
	if (num_agents < 1) {
		Logger::log("fetchConfigs", "Configuration file error: num_agents must be >= 1.");
		return {};
	}
	
	
	std::vector<AgentConfigs> configs(num_agents);
	for (size_t i = 0; i < num_agents; i++) {
		std::string config_path = std::string("Agent") + std::to_string(i) + std::string(".");
		configs[i].id = atoll(config_reader.readConfigValueString(config_path + std::string("id")).c_str());
		configs[i].device_mask = config_reader.readConfigValueString(config_path + std::string("device_mask"));
		configs[i].listen_port = atoll(config_reader.readConfigValueString(config_path + std::string("listen_port")).c_str());
		configs[i].device_abuse = (config_reader.readConfigValueString(config_path + std::string("device_abuse")) == std::string("true"));
		configs[i].user_data_directory = config_reader.readConfigValueString(config_path + std::string("user_data_directory"));
		configs[i].build_command = config_reader.readConfigValueString(config_path + std::string("build_command"));		
	}

	return configs;
}

int main(int argc, char* argv[]) {
	
	// what command ?
	std::string cmd;
	if (argc == 2)
		cmd = std::string(argv[1]);
	else
		cmd = "-run";

	// prepare
	LaunchModes launch_event;
	std::vector<AgentConfigs> configs;
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

		// reading launch configurations
		std::string config_file = pfacesAgentUtils::getApplicationDirectory() + "agent.conf";
		if (!pfacesFileIO::isFileExist(config_file)) {
			Logger::log(AGENT_MAIN, "Configuration file not found. Existing ... !");
			return -1;
		}
		Logger::log(AGENT_MAIN, "Reading the agents configs.");
		configs = fetchConfigs(config_file);
		if (configs.size() == 0) {
			Logger::log(AGENT_MAIN, "Failed to fetch the agent configurations. Existing ... !");
			return -1;
		}
	}
	else {
		Logger::log(AGENT_MAIN, 
			std::string("Invalid launch command (") + cmd +
			std::string("). pfaces-agent.exe must recieve one of the following launch commands: -install, -uninstall, or -run. Exiting ... !")
		);
		return -1;
	}

	// launch
	Logger::log(AGENT_MAIN, "Creating the service/daemon and calling its launch method.");
	pFacesAgentManager agentManager(configs);
	agentManager.launch(launch_event);
	
	
	// Maybe later:
	// here we can report to a centeral server that some agents are installed, uninstalled, ran or shut-down

	return 0;
}
#endif