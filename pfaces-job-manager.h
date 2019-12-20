#ifndef PFACES_JOB_MANAGER
#define PFACES_JOB_MANAGER

#include <string>
#include <vector>

// a class to maintain a running job
class pFacesJob {
	enum class PFACES_JOB_STATUS {
		idle,
		started,
		finished,
		killed
	};

	size_t id;
	std::string launch_cmd;
	PFACES_JOB_STATUS status;
	std::string output;
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
public:
	pfacesJobManager() = default;
	size_t launchJob(const std::string& _launch_cmd);
	void killJob(size_t id);
	std::string getJobOutput(size_t id);
	std::string getJobStatus(size_t id);

	std::string getJobsTableJSON(std::string id_col_name, std::string status_col_name, std::string details_col_name);
};

#endif // !PFACES_JOB
