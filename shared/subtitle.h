#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QDataStream>

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

inline QDataStream& operator<<(QDataStream& stream, const subforge::Subtitle& sub) {
    return stream << sub.id << sub.start_time << sub.end_time 
                  << QString::fromStdString(sub.text);
}

inline QDataStream& operator>>(QDataStream& stream, subforge::Subtitle& sub) {
    QString text;
    stream >> sub.id >> sub.start_time >> sub.end_time >> text;
    sub.text = text.toStdString();
    return stream;
}