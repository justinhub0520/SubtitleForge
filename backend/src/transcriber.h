#pragma once

#include "subtitle.h"
#include <string>
#include <vector>

namespace subforge {

class Transcriber {
public:
    Transcriber();

    std::vector<Subtitle> transcribe(const std::string& audio_path, const std::string& language = "auto");

private:
    std::string call_whisper_api(const std::string& audio_path, const std::string& language);
    std::vector<Subtitle> parse_whisper_response(const std::string& json_response);

    std::string api_key_;
};

} // namespace subforge