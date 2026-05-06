#pragma once

#include "task_status.h"
#include <string>
#include <map>
#include <mutex>
#include <functional>

namespace subforge {

class TaskManager {
public:
    static TaskManager& instance();

    std::string create_task(const std::string& video_id);
    void update_progress(const std::string& task_id, int progress);
    void mark_done(const std::string& task_id);
    void mark_error(const std::string& task_id, const std::string& error);
    TaskStatus get_status(const std::string& task_id);
    bool exists(const std::string& task_id) const;

private:
    TaskManager() = default;

    std::map<std::string, TaskStatus> tasks_;
    mutable std::mutex mutex_;

    std::string generate_id();
};

} // namespace subforge