#pragma once

#include <QWidget>

class QVideoWidget;
class QMediaPlayer;

class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget *parent = nullptr);

    void load_video(const QString& file_path);
    void play();
    void pause();
    void stop();
    void set_position(double seconds);
    double get_position() const;
    double get_duration() const;

signals:
    void position_changed(double seconds);
    void duration_changed(double seconds);
    void playback_finished();

private:
    QVideoWidget* video_widget_;
    QMediaPlayer* player_;
};