#pragma once

#include <string>

namespace subforge {

struct TaskStatus {
    std::string task_id;
    std::string video_id;
    int progress = 0;
    std::string status; // pending/processing/done/error
    std::string error_message;
    int eta_seconds = 0;
};

} // namespace subforge