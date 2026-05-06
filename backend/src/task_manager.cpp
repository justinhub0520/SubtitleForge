#include "task_manager.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace subforge {

TaskManager& TaskManager::instance() {
    static TaskManager inst;
    return inst;
}

std::string TaskManager::generate_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string TaskManager::create_task(const std::string& video_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string task_id = generate_id();
    TaskStatus status;
    status.task_id = task_id;
    status.video_id = video_id;
    status.progress = 0;
    status.status = "pending";
    tasks_[task_id] = status;

    return task_id;
}

void TaskManager::update_progress(const std::string& task_id, int progress) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        it->second.progress = progress;
        it->second.status = "processing";
    }
}

void TaskManager::mark_done(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        it->second.progress = 100;
        it->second.status = "done";
    }
}

void TaskManager::mark_error(const std::string& task_id, const std::string& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        it->second.status = "error";
        it->second.error_message = error;
    }
}

TaskStatus TaskManager::get_status(const std::string& task_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tasks_.find(task_id);
    if (it != tasks_.end()) {
        return it->second;
    }
    TaskStatus empty;
    empty.status = "not_found";
    return empty;
}

bool TaskManager::exists(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.find(task_id) != tasks_.end();
}

} // namespace subforge