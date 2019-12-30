#include "pfaces-agent-utils.h"
#include "pfaces-job-manager.h"


pFacesJob::pFacesJob(size_t _id, const std::string& _launch_cmd) {
	launch_cmd = _launch_cmd;
	id = _id;
	status = PFACES_JOB_STATUS::idle;

	output = std::make_shared<std::string>("");
	done_notifier = std::make_shared<bool>();
	*done_notifier = false;
	kill_signal = std::make_shared<bool>();
	*kill_signal = false;
	thread_mutex = std::make_shared<std::mutex>();
}
std::string pFacesJob::getLaunchCommand() {
	return launch_cmd;
}
size_t pFacesJob::getId() {
	return id;
}
std::string pFacesJob::statusStr() {
	switch (status)
	{
	case pFacesJob::PFACES_JOB_STATUS::idle:
		return "idle";
		break;
	case pFacesJob::PFACES_JOB_STATUS::started:
		return "started";
		break;
	case pFacesJob::PFACES_JOB_STATUS::finished:
		return "finished";
		break;
	case pFacesJob::PFACES_JOB_STATUS::killed:
		return "killed";
		break;
	default:
		return "error";
		break;
	}
}
std::string pFacesJob::outputStr() {
	thread_mutex->lock();
	return *output;
	thread_mutex->unlock();
}
void pFacesJob::refresh() {
	// TODO: upodate status + get latest output
}
void pFacesJob::run() {
	if (status == PFACES_JOB_STATUS::started || 
		status == PFACES_JOB_STATUS::finished || 
		status == PFACES_JOB_STATUS::killed)
		return;

	job_thread = std::make_shared<std::thread>(
			pfacesAgentUtils::exec_non_blocking_thread_ready,
			launch_cmd,
			output,	
			done_notifier,
			kill_signal,
			thread_mutex
		);

	status = PFACES_JOB_STATUS::started;
}
void pFacesJob::kill() {
	if (
		status == PFACES_JOB_STATUS::finished ||
		status == PFACES_JOB_STATUS::killed)
		return;

	if (status == PFACES_JOB_STATUS::idle){
		status = PFACES_JOB_STATUS::killed;
		return;
	}

	thread_mutex->lock();
	*kill_signal = true;
	thread_mutex->unlock();
	job_thread->join();

	status = PFACES_JOB_STATUS::killed;
}


size_t pfacesJobManager::launchJob(const std::string& _launch_cmd) {
	size_t new_id = pfacesJobs.size() + 1000;
	std::shared_ptr<pFacesJob> newJob = std::make_shared<pFacesJob>(new_id, _launch_cmd);
	pfacesJobs.push_back(newJob);
	try {
		newJob->run();
		return new_id;
	}
	catch (...) {
		return 0;
	}
}
void pfacesJobManager::killJob(size_t id) {
	// dont delete the job from the list
	// otherwise, you need to changine the IDing mechanism
	for (size_t i = 0; i < pfacesJobs.size(); i++){
		if (pfacesJobs[i]->getId() == id) {
			pfacesJobs[i]->kill();
			return;
		}
	}
}
std::string pfacesJobManager::getJobOutput(size_t id) {
	for (size_t i = 0; i < pfacesJobs.size(); i++) {
		if (pfacesJobs[i]->getId() == id) {
			return pfacesJobs[i]->outputStr();
		}
	}
	return "job-not-found";
}
std::string pfacesJobManager::getJobStatus(size_t id) {
	for (size_t i = 0; i < pfacesJobs.size(); i++) {
		if (pfacesJobs[i]->getId() == id) {
			return pfacesJobs[i]->statusStr();
		}
	}
	return "job-not-found";
}
std::string pfacesJobManager::getJobsTableJSON(
	std::string id_col_name, 
	std::string id_cmd_name,
	std::string status_col_name, 
	std::string details_col_name) {

	std::vector<std::string> table_head_keys = {
		id_col_name,
		id_cmd_name,
		status_col_name,
		details_col_name
	};
	std::vector<std::vector<std::string>> info_list;
	for (size_t i = 0; i < pfacesJobs.size(); i++){
		pfacesJobs[i]->refresh();
		std::vector<std::string> info;

		info.push_back(std::to_string(pfacesJobs[i]->getId()));
		info.push_back(pfacesJobs[i]->getLaunchCommand());
		info.push_back(pfacesJobs[i]->statusStr());
		info.push_back(pfacesJobs[i]->getLaunchCommand());
	}
	return (pfacesJobs.size() > 0) ?
		pfacesAgentUtils::makeJSONTable(table_head_keys, info_list) :
		"";
}