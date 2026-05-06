#pragma once

#include "video_info.h"
#include <string>

namespace subforge {

class AudioExtractor {
public:
    AudioExtractor();

    std::string extract(const std::string& video_path, const std::string& output_dir);
    VideoInfo get_video_info(const std::string& video_path);

private:
    std::string run_command(const std::string& cmd);

    std::string ffmpeg_path_;
    std::string ffprobe_path_;
};

} // namespace subforge