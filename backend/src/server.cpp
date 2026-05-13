#include "server.h"
#include "config.h"
#include "upload_handler.h"
#include "transcribe_handler.h"
#include "export_handler.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace subforge {

Server::Server() {
    setup_routes();
}

void Server::setup_routes() {
    server_.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        handle_health(req, res);
    });

    server_.Post("/api/videos/upload", [](const httplib::Request& req, httplib::Response& res) {
        UploadHandler::handle_upload(req, res);
    });

    server_.Post("/api/tasks/transcribe", [](const httplib::Request& req, httplib::Response& res) {
        TranscribeHandler::handle_transcribe(req, res);
    });

    server_.Get("/api/tasks/:task_id", [](const httplib::Request& req, httplib::Response& res) {
        TranscribeHandler::handle_task_status(req, res);
    });

    server_.Get("/api/videos/:video_id/subtitles", [](const httplib::Request& req, httplib::Response& res) {
        ExportHandler::handle_get_subtitles(req, res);
    });

    server_.Post("/api/tasks/export", [](const httplib::Request& req, httplib::Response& res) {
        ExportHandler::handle_export_video(req, res);
    });

    server_.Get("/api/videos/:video_id/download", [](const httplib::Request& req, httplib::Response& res) {
        ExportHandler::handle_download(req, res);
    });

    server_.Get("/api/videos/:video_id/audio", [](const httplib::Request& req, httplib::Response& res) {
        std::string video_id = req.path_params.at("video_id");
        auto& config = Config::instance();
        std::string audio_path = config.temp_dir() + "/" + video_id + "/audio.wav";

        std::ifstream file(audio_path, std::ios::binary);
        if (!file.is_open()) {
            res.status = 404;
            json err = {{"error", "Audio file not found"}};
            res.set_content(err.dump(), "application/json");
            return;
        }

        std::string data((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        res.set_content(data, "audio/wav");
    });
}

void Server::handle_health(const httplib::Request& req, httplib::Response& res) {
    json response = {
        {"status", "ok"},
        {"service", "SubForge Backend"},
        {"version", "1.0.0"}
    };
    res.set_content(response.dump(), "application/json");
}

void Server::start(const std::string& host, int port) {
    std::cout << "Starting SubForge backend on " << host << ":" << port << std::endl;
    running_ = true;
    server_.listen(host, port);
}

void Server::stop() {
    running_ = false;
    server_.stop();
}

} // namespace subforge