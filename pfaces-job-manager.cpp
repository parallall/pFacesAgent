#include "pfaces-agent-utils.h"
#include "pfaces-job-manager.h"


pFacesJob::pFacesJob(size_t _id, const std::string& _launch_cmd) {
	control_pack = std::make_shared<non_blocking_thread_pack>();
	control_pack->launch_cmd = _launch_cmd;
	id = _id;
	status = PFACES_JOB_STATUS::idle;
}
std::string pFacesJob::getLaunchCommand() {
	return control_pack->launch_cmd;
}
size_t pFacesJob::getId() {
	return id;
}
std::string pFacesJob::statusStr() {
	switch (status)
	{
	case PFACES_JOB_STATUS::idle:
		return "idle";
		break;
	case PFACES_JOB_STATUS::started:
		return "started";
		break;
	case PFACES_JOB_STATUS::finished:
		return "finished";
		break;
	case PFACES_JOB_STATUS::killed:
		return "killed";
		break;
	default:
		return "error";
		break;
	}
}
std::string pFacesJob::outputStr() {
	std::lock_guard<std::mutex> lock(control_pack->thread_mutex);
	return control_pack->output;
}
void pFacesJob::refresh() {
	std::lock_guard<std::mutex> lock(control_pack->thread_mutex);
	if (control_pack->done_notifier)
		status = PFACES_JOB_STATUS::finished;
}
void pFacesJob::run() {
	if (status == PFACES_JOB_STATUS::started || 
		status == PFACES_JOB_STATUS::finished || 
		status == PFACES_JOB_STATUS::killed)
		return;

	job_thread = std::thread(
		pfacesAgentUtils::exec_non_blocking_thread_ready,
		control_pack
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

	{
		std::lock_guard<std::mutex> lock(control_pack->thread_mutex);
		control_pack->kill_signal = true;
	}
	job_thread.join();

	status = PFACES_JOB_STATUS::killed;
}


size_t pfacesJobManager::launchJob(const std::string& _launch_cmd) {
	std::lock_guard<std::mutex> lock(work_coordinator);
	size_t new_id = pfacesJobs.size() + 1000;
	pfacesJobs.push_back(std::make_shared<pFacesJob>(new_id, _launch_cmd));
	size_t ret;
	try {
		pfacesJobs[pfacesJobs.size()-1]->run();
		ret = new_id;
	}
	catch (...) {
		ret = 0;
	}
	return ret;
}
void pfacesJobManager::killJob(size_t id) {
	std::lock_guard<std::mutex> lock(work_coordinator);
	// dont delete the job from the list
	// otherwise, you need to changine the IDing mechanism
	for (size_t i = 0; i < pfacesJobs.size(); i++){
		if (pfacesJobs[i]->getId() == id) {
			pfacesJobs[i]->refresh();
			pfacesJobs[i]->kill();
			break;
		}
	}
}
std::string pfacesJobManager::getJobOutput(size_t id) {
	std::lock_guard<std::mutex> lock(work_coordinator);
	std::string ret;
	for (size_t i = 0; i < pfacesJobs.size(); i++) {
		if (pfacesJobs[i]->getId() == id) {
			pfacesJobs[i]->refresh();
			ret = pfacesJobs[i]->outputStr();
			break;
		}
	}
	if (ret.empty())
		return "job-not-found";
	else
		return ret;
}
std::string pfacesJobManager::getJobStatus(size_t id) {
	std::lock_guard<std::mutex> lock(work_coordinator);
	std::string ret;
	for (size_t i = 0; i < pfacesJobs.size(); i++) {
		if (pfacesJobs[i]->getId() == id) {
			pfacesJobs[i]->refresh();
			ret = pfacesJobs[i]->statusStr();
			break;
		}
	}
	if (ret.empty())
		return "job-not-found";
	else
		return ret;
}
std::string pfacesJobManager::getJobsTableJSON(
	std::string id_col_name, 
	std::string id_cmd_name,
	std::string status_col_name, 
	std::string details_col_name) {
	
	std::lock_guard<std::mutex> lock(work_coordinator);
	size_t num_jobs = pfacesJobs.size();
	std::vector<std::string> table_head_keys = {
		id_col_name,
		id_cmd_name,
		status_col_name,
		details_col_name
	};
	std::vector<std::vector<std::string>> info_list;
	for (size_t i = 0; i < num_jobs; i++){
		pfacesJobs[i]->refresh();
		std::vector<std::string> info;

		info.push_back(std::to_string(pfacesJobs[i]->getId()));
		info.push_back(pfacesJobs[i]->getLaunchCommand());
		info.push_back(pfacesJobs[i]->statusStr());
		info.push_back(pfacesJobs[i]->outputStr());

		info_list.push_back(info);
	}
	return (num_jobs > 0) ?
		pfacesAgentUtils::makeJSONTable(table_head_keys, info_list) :
		"no-jobs";
}