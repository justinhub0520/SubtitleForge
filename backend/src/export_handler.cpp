#include "export_handler.h"
#include "config.h"
#include "srt_parser.h"
#include "subtitle.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <array>
#include <memory>
#include <sys/stat.h>

using json = nlohmann::json;

namespace subforge {

static std::string run_ffmpeg_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void ExportHandler::handle_get_subtitles(const httplib::Request& req, httplib::Response& res) {
    std::string video_id = req.path_params.at("video_id");
    auto& config = Config::instance();
    std::string json_path = config.temp_dir() + "/" + video_id + "/subtitles.json";

    std::ifstream file(json_path);
    if (!file.is_open()) {
        res.status = 404;
        json err = {{"error", "Subtitles not found"}};
        res.set_content(err.dump(), "application/json");
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    res.set_content(buffer.str(), "application/json");
}

void ExportHandler::handle_export_video(const httplib::Request& req, httplib::Response& res) {
    try {
        json body = json::parse(req.body);
        std::string video_id = body.value("video_id", "");

        if (video_id.empty()) {
            res.status = 400;
            json err = {{"error", "video_id is required"}};
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto& config = Config::instance();
        std::string video_path = config.upload_dir() + "/" + video_id + ".mp4";
        std::string srt_path = config.temp_dir() + "/" + video_id + "/subtitles.srt";
        std::string output_path = config.temp_dir() + "/" + video_id + "/output.mp4";

        std::string escaped_srt = srt_path;
        std::string cmd = config.ffmpeg_path()
            + " -y -i \"" + video_path + "\""
            + " -vf \"subtitles='" + escaped_srt + "'\""
            + " -c:a copy \"" + output_path + "\" 2>&1";

        std::string result = run_ffmpeg_command(cmd);

        struct stat st;
        if (stat(output_path.c_str(), &st) != 0 || st.st_size == 0) {
            res.status = 500;
            json err = {{"error", "FFmpeg export failed"}, {"details", result.substr(0, 300)}};
            res.set_content(err.dump(), "application/json");
            return;
        }

        json response = {
            {"video_id", video_id},
            {"file_path", output_path},
            {"message", "Export successful"}
        };
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 500;
        json err = {{"error", e.what()}};
        res.set_content(err.dump(), "application/json");
    }
}

void ExportHandler::handle_download(const httplib::Request& req, httplib::Response& res) {
    std::string video_id = req.path_params.at("video_id");
    auto& config = Config::instance();
    std::string output_path = config.temp_dir() + "/" + video_id + "/output.mp4";

    std::ifstream file(output_path, std::ios::binary);
    if (!file.is_open()) {
        res.status = 404;
        json err = {{"error", "Exported video not found"}};
        res.set_content(err.dump(), "application/json");
        return;
    }

    std::string data((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

    res.set_content(data, "video/mp4");
    res.set_header("Content-Disposition", "attachment; filename=\"" + video_id + "_subtitled.mp4\"");
}

} // namespace subforge