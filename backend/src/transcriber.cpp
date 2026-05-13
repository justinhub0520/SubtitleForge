#include "transcriber.h"
#include "config.h"
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace subforge {

Transcriber::Transcriber() {
    api_key_ = Config::instance().dashscope_api_key();
    if (api_key_.empty()) {
        throw std::runtime_error("DASHSCOPE_API_KEY not configured");
    }
}

static std::string shell_escape(const std::string& s) {
    std::string result;
    result.reserve(s.size() + 4);
    result += '\'';
    for (char c : s) {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    result += '\'';
    return result;
}

static std::pair<int, std::string> run_curl(const std::string& cmd) {
    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) throw std::runtime_error("curl returned empty response");

    size_t last_newline = result.rfind('\n');
    if (last_newline == std::string::npos)
        throw std::runtime_error("curl unexpected output: " + result.substr(0, 200));

    std::string http_code_str = result.substr(last_newline + 1);
    std::string body_response = result.substr(0, last_newline);

    int http_code = 0;
    try { http_code = std::stoi(http_code_str); }
    catch (...) { throw std::runtime_error("curl bad status: " + http_code_str); }

    return {http_code, body_response};
}

static std::pair<int, std::string> run_curl_with_retry(const std::string& cmd, int max_retries = 5) {
    int backoff_seconds = 2;
    std::pair<int, std::string> last_result = {0, ""};

    for (int attempt = 0; attempt <= max_retries; ++attempt) {
        if (attempt > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(backoff_seconds));
            backoff_seconds = std::min(backoff_seconds * 2, 60);
        }
        last_result = run_curl(cmd);
        if (last_result.first != 429) break;
    }
    return last_result;
}

std::string Transcriber::http_get(const std::string& url,
                                   const std::vector<std::pair<std::string, std::string>>& headers) {
    std::string cmd = "/usr/bin/curl -s -w '\\n%{http_code}' ";
    for (const auto& h : headers) {
        cmd += " -H " + shell_escape(h.first + ": " + h.second);
    }
    cmd += " --connect-timeout 30 --max-time 60 " + shell_escape(url) + " 2>&1";

    auto [http_code, body] = run_curl_with_retry(cmd);
    if (http_code != 200) {
        throw std::runtime_error("HTTP GET " + std::to_string(http_code) + ": " + body.substr(0, 300));
    }
    return body;
}

std::string Transcriber::curl_upload_file(const std::string& url, const std::string& file_path,
                                           const std::vector<std::pair<std::string, std::string>>& headers) {
    std::string cmd = "/usr/bin/curl -s -w '\\n%{http_code}' ";
    for (const auto& h : headers) {
        cmd += " -H " + shell_escape(h.first + ": " + h.second);
    }
    cmd += " -F file=@" + shell_escape(file_path);
    cmd += " --connect-timeout 60 --max-time 120 " + shell_escape(url) + " 2>&1";

    auto [http_code, body] = run_curl_with_retry(cmd);
    if (http_code != 200) {
        throw std::runtime_error("File upload HTTP " + std::to_string(http_code) + ": " + body.substr(0, 300));
    }
    return body;
}

std::string Transcriber::upload_audio_file(const std::string& file_path) {
    std::vector<std::pair<std::string, std::string>> headers = {
        {"Authorization", "Bearer " + api_key_}
    };

    std::string response = curl_upload_file(
        "https://dashscope.aliyuncs.com/api/v1/files", file_path, headers);

    json resp = json::parse(response);
    if (resp.contains("data") && resp["data"].contains("uploaded_files")) {
        auto& files = resp["data"]["uploaded_files"];
        if (files.is_array() && !files.empty() && files[0].contains("file_id")) {
            return files[0]["file_id"].get<std::string>();
        }
    }
    throw std::runtime_error("File upload failed: no file_id - " + response.substr(0, 300));
}

std::string Transcriber::resolve_file_url(const std::string& file_id) {
    std::vector<std::pair<std::string, std::string>> headers = {
        {"Authorization", "Bearer " + api_key_}
    };

    std::string response = http_get(
        "https://dashscope.aliyuncs.com/api/v1/files/" + file_id, headers);

    json resp = json::parse(response);
    if (resp.contains("data") && resp["data"].contains("url")) {
        return resp["data"]["url"].get<std::string>();
    }
    throw std::runtime_error("Failed to resolve file URL: " + response.substr(0, 300));
}

std::string Transcriber::submit_task(const std::string& file_url, const std::string& language) {
    json body;
    body["model"] = "fun-asr";
    body["input"]["file_urls"] = json::array({file_url});
    body["parameters"]["language_hints"] = json::array({language});

    std::string cmd = "/usr/bin/curl -s -w '\\n%{http_code}' -X POST "
        "-H " + shell_escape("Authorization: Bearer " + api_key_) + " "
        "-H " + shell_escape("Content-Type: application/json") + " "
        "-H " + shell_escape("X-DashScope-Async: enable") + " "
        "-d " + shell_escape(body.dump()) + " "
        "--connect-timeout 60 --max-time 300 "
        + shell_escape("https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription")
        + " 2>&1";

    auto [http_code, response] = run_curl(cmd);
    if (http_code != 200) {
        throw std::runtime_error("Submit failed HTTP " + std::to_string(http_code) + ": " + response.substr(0, 300));
    }

    json resp = json::parse(response);
    if (resp.contains("output") && resp["output"].contains("task_id")) {
        return resp["output"]["task_id"];
    }
    throw std::runtime_error("No task_id in response: " + response.substr(0, 300));
}

std::string Transcriber::wait_for_result(const std::string& task_id) {
    std::vector<std::pair<std::string, std::string>> headers = {
        {"Authorization", "Bearer " + api_key_}
    };

    for (int i = 0; i < 120; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::string response = http_get(
            "https://dashscope.aliyuncs.com/api/v1/tasks/" + task_id, headers);

        json resp = json::parse(response);
        std::string status = resp["output"].value("task_status", "");

        if (status == "SUCCEEDED") {
            if (resp["output"].contains("results")) {
                for (const auto& result : resp["output"]["results"]) {
                    if (result.contains("transcription_url")) {
                        return http_get(result["transcription_url"].get<std::string>(), {});
                    }
                }
            }
            throw std::runtime_error("Task succeeded but no transcription_url found");
        }
        if (status == "FAILED") {
            throw std::runtime_error("Transcription task failed: "
                + resp["output"].value("message", "Unknown error"));
        }
    }
    throw std::runtime_error("Transcription task timeout after 10 minutes");
}

std::vector<Subtitle> Transcriber::parse_result(const std::string& result_json) {
    json data = json::parse(result_json);
    std::vector<Subtitle> subtitles;

    if (data.contains("transcripts") && data["transcripts"].is_array()) {
        for (const auto& transcript : data["transcripts"]) {
            if (transcript.contains("sentences") && transcript["sentences"].is_array()) {
                for (const auto& sentence : transcript["sentences"]) {
                    Subtitle sub;
                    sub.id = static_cast<int>(subtitles.size()) + 1;
                    sub.start_time = sentence.value("begin_time", 0.0) / 1000.0;
                    sub.end_time = sentence.value("end_time", 0.0) / 1000.0;
                    sub.text = sentence.value("text", "");
                    subtitles.push_back(sub);
                }
            }
        }
    }
    return subtitles;
}

std::vector<Subtitle> Transcriber::transcribe(const std::string& audio_file_path, const std::string& language) {
    std::string file_id = upload_audio_file(audio_file_path);
    std::string file_url = resolve_file_url(file_id);
    std::string task_id = submit_task(file_url, language);
    std::string result_json = wait_for_result(task_id);
    return parse_result(result_json);
}

} // namespace subforge