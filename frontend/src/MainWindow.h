#pragma once

#include <QMainWindow>
#include <QList>
#include "subtitle.h"

class VideoPlayerWidget;
class TimelineEditor;
class SubtitleListWidget;
class StyleEditorWidget;
class ApiClient;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void on_open_video();
    void on_upload_video();
    void on_generate_subtitles();
    void on_export_video();
    void on_export_srt();
    void on_subtitle_selected(int id);
    void on_subtitle_changed(const QList<subforge::Subtitle>& subtitles);
    void on_time_requested(double seconds);
    void on_position_changed(double seconds);
    void on_duration_changed(double seconds);
    void on_style_changed(const subforge::SubtitleStyle& style);

private:
    void setup_ui();
    void setup_menu();
    void setup_toolbar();
    void connect_signals();
    void update_status_bar(const QString& message);

    VideoPlayerWidget* video_player_;
    TimelineEditor* timeline_;
    SubtitleListWidget* subtitle_list_;
    StyleEditorWidget* style_editor_;
    ApiClient* api_client_;

    QList<subforge::Subtitle> current_subtitles_;
    QString current_video_path_;
    QString current_video_id_;
    double video_duration_ = 0.0;
};