#include "config.h"
#include <fstream>
#include <cstdlib>

namespace subforge {

Config& Config::instance() {
    static Config inst;
    return inst;
}

Config::Config() {
    load_env_file();
}

void Config::load_env_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        values_[key] = value;
    }
}

std::string Config::get(const std::string& key, const std::string& default_val) const {
    auto it = values_.find(key);
    if (it != values_.end()) return it->second;

    const char* env = std::getenv(key.c_str());
    if (env) return env;

    return default_val;
}

int Config::get_int(const std::string& key, int default_val) const {
    std::string val = get(key);
    if (val.empty()) return default_val;
    try { return std::stoi(val); }
    catch (...) { return default_val; }
}

std::string Config::whisper_api_key() const { return get("WHISPER_API_KEY"); }
std::string Config::server_host() const { return get("SERVER_HOST", "0.0.0.0"); }
int Config::server_port() const { return get_int("SERVER_PORT", 8080); }
std::string Config::ffmpeg_path() const { return get("FFMPEG_PATH", "/usr/bin/ffmpeg"); }
std::string Config::ffprobe_path() const { return get("FFPROBE_PATH", "/usr/bin/ffprobe"); }
std::string Config::temp_dir() const { return get("TEMP_DIR", "/tmp/subforge"); }
std::string Config::upload_dir() const { return get("UPLOAD_DIR", "/var/lib/subforge/uploads"); }

} // namespace subforge