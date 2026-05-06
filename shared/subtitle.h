#pragma once

#include <string>
#include <vector>

namespace subforge {

struct SubtitleStyle {
    std::string font_family = "Microsoft YaHei";
    int font_size = 24;
    std::string color = "#FFFFFF";
    std::string bg_color = "#00000080";
    std::string position = "bottom_center";
    bool bold = false;
    bool italic = false;
    int outline_width = 2;
    std::string outline_color = "#000000";
};

struct Subtitle {
    int id;
    double start_time;
    double end_time;
    std::string text;
    SubtitleStyle style;
};

} // namespace subforge