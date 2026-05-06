#include "audio_extractor.h"
#include "config.h"
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace subforge {

AudioExtractor::AudioExtractor() {
    auto& config = Config::instance();
    ffmpeg_path_ = config.ffmpeg_path();
    ffprobe_path_ = config.ffprobe_path();
}

std::string AudioExtractor::run_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string AudioExtractor::extract(const std::string& video_path, const std::string& output_dir) {
    mkdir(output_dir.c_str(), 0755);

    std::string audio_path = output_dir + "/audio.wav";
    std::string cmd = ffmpeg_path_ + " -y -i \"" + video_path
        + "\" -vn -acodec pcm_s16le -ar 16000 -ac 1 \"" + audio_path + "\" 2>&1";

    std::string output = run_command(cmd);

    struct stat st;
    if (stat(audio_path.c_str(), &st) != 0 || st.st_size == 0) {
        throw std::runtime_error("Audio extraction failed: " + output.substr(0, 200));
    }

    return audio_path;
}

VideoInfo AudioExtractor::get_video_info(const std::string& video_path) {
    std::string cmd = ffprobe_path_
        + " -v quiet -print_format json -show_format -show_streams \"" + video_path + "\" 2>&1";

    std::string output = run_command(cmd);
    json probe = json::parse(output);

    VideoInfo info;
    info.file_path = video_path;

    for (const auto& stream : probe["streams"]) {
        if (stream["codec_type"] == "video") {
            info.width = stream.value("width", 0);
            info.height = stream.value("height", 0);
            std::string fps_str = stream.value("r_frame_rate", "30/1");
            size_t slash_pos = fps_str.find('/');
            if (slash_pos != std::string::npos) {
                int num = std::stoi(fps_str.substr(0, slash_pos));
                int den = std::stoi(fps_str.substr(slash_pos + 1));
                info.fps = den > 0 ? num / den : 30;
            }
        }
    }

    if (probe.contains("format")) {
        info.duration = probe["format"].value("duration", 0.0);
        double size_bytes = probe["format"].value("size", 0.0);
        info.size_mb = size_bytes / (1024.0 * 1024.0);
        info.format = probe["format"].value("format_name", "unknown");
    }

    return info;
}

} // namespace subforge