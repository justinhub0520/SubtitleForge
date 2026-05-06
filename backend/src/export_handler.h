#pragma once

#include "httplib.h"

namespace subforge {

class ExportHandler {
public:
    static void handle_export_video(const httplib::Request& req, httplib::Response& res);
    static void handle_get_subtitles(const httplib::Request& req, httplib::Response& res);
    static void handle_download(const httplib::Request& req, httplib::Response& res);
};

} // namespace subforge