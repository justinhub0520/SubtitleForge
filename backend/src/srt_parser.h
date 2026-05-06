#pragma once

#include "subtitle.h"
#include <string>
#include <vector>

namespace subforge {

class SrtParser {
public:
    static std::vector<Subtitle> parse(const std::string& srt_content);
    static std::string to_srt(const std::vector<Subtitle>& subtitles);

private:
    static double parse_timestamp(const std::string& timestamp);
    static std::string format_timestamp(double seconds);
};

} // namespace subforge