#ifndef PFACES_JOB_MANAGER
#define PFACES_JOB_MANAGER

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include "pfaces-agent-utils.h"

// a class to maintain a running job
enum class PFACES_JOB_STATUS {
	idle,
	started,
	finished,
	killed
};
class pFacesJob {
	size_t id;
	PFACES_JOB_STATUS status;	
	std::thread job_thread;	
	std::shared_ptr<non_blocking_thread_pack> control_pack;

public:
	pFacesJob(size_t _id, const std::string& _launch_cmd);

	std::string getLaunchCommand();
	size_t getId();
	std::string statusStr();
	std::string outputStr();
	void refresh();
	void run();
	void kill();
};


class pfacesJobManager {
	std::vector<std::shared_ptr<pFacesJob>> pfacesJobs;
	std::mutex work_coordinator;
public:
	pfacesJobManager() = default;
	size_t launchJob(const std::string& _launch_cmd);
	void killJob(size_t id);
	std::string getJobOutput(size_t id);
	std::string getJobStatus(size_t id);

	std::string getJobsTableJSON(
		std::string id_col_name, 
		std::string id_cmd_name,
		std::string status_col_name, 
		std::string details_col_name);
};

#endif // !PFACES_JOB
