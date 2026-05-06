#pragma once

#include "httplib.h"

namespace subforge {

class TranscribeHandler {
public:
    static void handle_transcribe(const httplib::Request& req, httplib::Response& res);
    static void handle_task_status(const httplib::Request& req, httplib::Response& res);
};

} // namespace subforge