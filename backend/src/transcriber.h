#pragma once

#include "subtitle.h"
#include <string>
#include <vector>

namespace subforge {

class Transcriber {
public:
    Transcriber();

    std::vector<Subtitle> transcribe(const std::string& audio_file_path, const std::string& language = "zh");

private:
    std::string upload_audio_file(const std::string& file_path);
    std::string resolve_file_url(const std::string& file_id);
    std::string submit_task(const std::string& file_url, const std::string& language);
    std::string wait_for_result(const std::string& task_id);
    std::vector<Subtitle> parse_result(const std::string& result_json);
    std::string http_get(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers);
    std::string curl_upload_file(const std::string& url, const std::string& file_path,
                                  const std::vector<std::pair<std::string, std::string>>& headers);

    std::string api_key_;
};

} // namespace subforge