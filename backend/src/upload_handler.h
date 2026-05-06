#pragma once

#include "httplib.h"
#include <string>

namespace subforge {

class UploadHandler {
public:
    static void handle_upload(const httplib::Request& req, httplib::Response& res);
    static std::string save_uploaded_file(const std::string& data, const std::string& filename, const std::string& upload_dir);
    static std::string generate_video_id();
};

} // namespace subforge