#pragma once

#include <string>

namespace subforge {

struct VideoInfo {
    std::string video_id;
    std::string file_path;
    std::string audio_path;
    double duration = 0.0;
    int width = 0;
    int height = 0;
    int fps = 0;
    std::string format;
    double size_mb = 0.0;
};

} // namespace subforge