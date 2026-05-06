#pragma once

#include <string>
#include <map>

namespace subforge {

class Config {
public:
    static Config& instance();

    std::string get(const std::string& key, const std::string& default_val = "") const;
    int get_int(const std::string& key, int default_val = 0) const;

    std::string whisper_api_key() const;
    std::string server_host() const;
    int server_port() const;
    std::string ffmpeg_path() const;
    std::string ffprobe_path() const;
    std::string temp_dir() const;
    std::string upload_dir() const;

private:
    Config();
    void load_env_file(const std::string& path = ".env");

    std::map<std::string, std::string> values_;
};

} // namespace subforge