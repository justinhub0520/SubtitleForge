#include "srt_parser.h"
#include <sstream>
#include <iomanip>
#include <cstdio>

namespace subforge {

double SrtParser::parse_timestamp(const std::string& timestamp) {
    int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    char comma_or_dot = '.';
    sscanf(timestamp.c_str(), "%d:%d:%d%c%d", &hours, &minutes, &seconds, &comma_or_dot, &milliseconds);

    return hours * 3600.0 + minutes * 60.0 + seconds + milliseconds / 1000.0;
}

std::string SrtParser::format_timestamp(double seconds) {
    int total_seconds = static_cast<int>(seconds);
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int secs = total_seconds % 60;
    int ms = static_cast<int>((seconds - total_seconds) * 1000);

    char buf[20];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d,%03d", hours, minutes, secs, ms);
    return buf;
}

std::vector<Subtitle> SrtParser::parse(const std::string& srt_content) {
    std::vector<Subtitle> subtitles;
    std::istringstream stream(srt_content);
    std::string line;

    while (std::getline(stream, line)) {
        Subtitle sub;

        std::getline(stream, line);
        if (line.empty()) continue;

        size_t arrow_pos = line.find("-->");
        if (arrow_pos == std::string::npos) continue;

        std::string start_str = line.substr(0, arrow_pos);
        std::string end_str = line.substr(arrow_pos + 3);

        sub.start_time = parse_timestamp(start_str);
        sub.end_time = parse_timestamp(end_str);

        std::string text;
        while (std::getline(stream, line) && !line.empty()) {
            if (!text.empty()) text += "\n";
            text += line;
        }
        sub.text = text;
        sub.id = static_cast<int>(subtitles.size()) + 1;

        subtitles.push_back(sub);
    }

    return subtitles;
}

std::string SrtParser::to_srt(const std::vector<Subtitle>& subtitles) {
    std::ostringstream stream;

    for (size_t i = 0; i < subtitles.size(); ++i) {
        const auto& sub = subtitles[i];
        stream << (i + 1) << "\n";
        stream << format_timestamp(sub.start_time) << " --> " << format_timestamp(sub.end_time) << "\n";
        stream << sub.text << "\n\n";
    }

    return stream.str();
}

} // namespace subforge