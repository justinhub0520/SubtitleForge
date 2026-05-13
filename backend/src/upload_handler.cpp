#include "upload_handler.h"
#include "config.h"
#include "task_manager.h"
#include "video_info.h"
#include "audio_extractor.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>
#include <sstream>
#include <sys/stat.h>

using json = nlohmann::json;

namespace subforge {

std::string UploadHandler::generate_video_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string UploadHandler::save_uploaded_file(const std::string& data, const std::string& filename, const std::string& upload_dir) {
    std::string video_id = generate_video_id();
    mkdir(upload_dir.c_str(), 0755);

    std::string file_path = upload_dir + "/" + video_id + ".mp4";
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + file_path);
    }

    file.write(data.data(), data.size());
    file.close();

    return video_id;
}

void UploadHandler::handle_upload(const httplib::Request& req, httplib::Response& res) {
    try {
        auto& config = Config::instance();
        std::string upload_dir = config.upload_dir();

        std::cerr << "[Upload] Received upload request, has_file: " << req.has_file("video") << std::endl;

        if (!req.has_file("video")) {
            std::cerr << "[Upload] No video file in request. Files: ";
            for (auto& [name, file] : req.files) {
                std::cerr << name << "=" << file.filename << " ";
            }
            std::cerr << std::endl;
            res.status = 400;
            json err = {{"error", "No video file provided"}};
            res.set_content(err.dump(), "application/json");
            return;
        }

        const auto& file = req.get_file_value("video");
        std::cerr << "[Upload] File: " << file.filename << " size: " << file.content.size() << std::endl;

        std::string video_id = save_uploaded_file(file.content, file.filename, upload_dir);
        std::cerr << "[Upload] Saved as: " << upload_dir << "/" << video_id << ".mp4" << std::endl;

        std::string video_path = upload_dir + "/" + video_id + ".mp4";

        AudioExtractor extractor;
        std::cerr << "[Upload] Getting video info with ffprobe: " << config.ffprobe_path() << std::endl;
        VideoInfo info = extractor.get_video_info(video_path);
        info.video_id = video_id;

        std::cerr << "[Upload] Video info: " << info.duration << "s, " << info.width << "x" << info.height << std::endl;

        json response = {
            {"video_id", video_id},
            {"message", "Upload successful"},
            {"duration", info.duration},
            {"width", info.width},
            {"height", info.height},
            {"fps", info.fps},
            {"format", info.format}
        };
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        std::cerr << "[Upload] ERROR: " << e.what() << std::endl;
        res.status = 500;
        json err = {{"error", e.what()}};
        res.set_content(err.dump(), "application/json");
    }
}

} // namespace subforge