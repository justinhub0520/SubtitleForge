#include "transcriber.h"
#include "config.h"
#include <httplib.h>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace subforge {

Transcriber::Transcriber() {
    api_key_ = Config::instance().whisper_api_key();
    if (api_key_.empty()) {
        throw std::runtime_error("WHISPER_API_KEY not configured");
    }
}

std::string Transcriber::call_whisper_api(const std::string& audio_path, const std::string& language) {
    httplib::Client cli("https://api.openai.com");
    cli.set_read_timeout(300);

    std::ifstream audio_file(audio_path, std::ios::binary);
    if (!audio_file.is_open()) {
        throw std::runtime_error("Cannot open audio file: " + audio_path);
    }

    std::string audio_data((std::istreambuf_iterator<char>(audio_file)),
                            std::istreambuf_iterator<char>());
    audio_file.close();

    httplib::MultipartFormDataItems items = {
        {"file", audio_data, audio_path, "audio/wav"},
        {"model", "whisper-1", "", "text/plain"},
        {"response_format", "verbose_json", "", "text/plain"},
        {"timestamp_granularities[]", "segment", "", "text/plain"},
    };

    if (language != "auto") {
        items.push_back({"language", language, "", "text/plain"});
    }

    httplib::Headers headers = {
        {"Authorization", "Bearer " + api_key_}
    };

    auto res = cli.Post("/v1/audio/transcriptions", headers, items);

    if (!res) {
        throw std::runtime_error("Whisper API request failed: " + httplib::to_string(res.error()));
    }

    if (res->status != 200) {
        throw std::runtime_error("Whisper API error (HTTP " + std::to_string(res->status) + "): " + res->body.substr(0, 300));
    }

    return res->body;
}

std::vector<Subtitle> Transcriber::parse_whisper_response(const std::string& json_response) {
    json data = json::parse(json_response);
    std::vector<Subtitle> subtitles;

    if (data.contains("segments")) {
        int id = 1;
        for (const auto& seg : data["segments"]) {
            Subtitle sub;
            sub.id = id++;
            sub.start_time = seg.value("start", 0.0);
            sub.end_time = seg.value("end", 0.0);
            sub.text = seg.value("text", "");
            subtitles.push_back(sub);
        }
    }

    return subtitles;
}

std::vector<Subtitle> Transcriber::transcribe(const std::string& audio_path, const std::string& language) {
    std::string response = call_whisper_api(audio_path, language);
    return parse_whisper_response(response);
}

} // namespace subforge