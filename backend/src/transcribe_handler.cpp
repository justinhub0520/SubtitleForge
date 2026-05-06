#include "transcribe_handler.h"
#include "config.h"
#include "audio_extractor.h"
#include "transcriber.h"
#include "task_manager.h"
#include "srt_parser.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <fstream>
#include <sys/stat.h>

using json = nlohmann::json;

namespace subforge {

void TranscribeHandler::handle_transcribe(const httplib::Request& req, httplib::Response& res) {
    try {
        json body = json::parse(req.body);
        std::string video_id = body.value("video_id", "");
        std::string language = body.value("language", "auto");

        if (video_id.empty()) {
            res.status = 400;
            json err = {{"error", "video_id is required"}};
            res.set_content(err.dump(), "application/json");
            return;
        }

        auto& config = Config::instance();
        std::string video_path = config.upload_dir() + "/" + video_id + ".mp4";
        std::string temp_dir = config.temp_dir() + "/" + video_id;

        std::string task_id = TaskManager::instance().create_task(video_id);

        std::thread([video_path, temp_dir, language, task_id]() {
            auto& tm = TaskManager::instance();
            try {
                tm.update_progress(task_id, 10);

                AudioExtractor extractor;
                mkdir(temp_dir.c_str(), 0755);
                std::string audio_path = extractor.extract(video_path, temp_dir);
                tm.update_progress(task_id, 40);

                Transcriber transcriber;
                auto subtitles = transcriber.transcribe(audio_path, language);
                tm.update_progress(task_id, 80);

                std::string srt_content = SrtParser::to_srt(subtitles);
                std::ofstream srt_file(temp_dir + "/subtitles.srt");
                srt_file << srt_content;
                srt_file.close();

                json subs_json = json::array();
                for (const auto& sub : subtitles) {
                    subs_json.push_back({
                        {"id", sub.id},
                        {"start_time", sub.start_time},
                        {"end_time", sub.end_time},
                        {"text", sub.text}
                    });
                }
                std::ofstream subs_file(temp_dir + "/subtitles.json");
                subs_file << subs_json.dump();
                subs_file.close();

                tm.update_progress(task_id, 95);
                tm.mark_done(task_id);
            } catch (const std::exception& e) {
                tm.mark_error(task_id, e.what());
            }
        }).detach();

        json response = {
            {"task_id", task_id},
            {"status", "processing"},
            {"message", "Transcription started"}
        };
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 500;
        json err = {{"error", e.what()}};
        res.set_content(err.dump(), "application/json");
    }
}

void TranscribeHandler::handle_task_status(const httplib::Request& req, httplib::Response& res) {
    std::string task_id = req.path_params.at("task_id");

    TaskStatus status = TaskManager::instance().get_status(task_id);

    json response = {
        {"task_id", status.task_id},
        {"video_id", status.video_id},
        {"progress", status.progress},
        {"status", status.status},
        {"error_message", status.error_message}
    };
    res.set_content(response.dump(), "application/json");
}

} // namespace subforge