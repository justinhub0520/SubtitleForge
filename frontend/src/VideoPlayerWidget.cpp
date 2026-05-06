#include "VideoPlayerWidget.h"
#include <QVBoxLayout>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>

VideoPlayerWidget::VideoPlayerWidget(QWidget *parent) : QWidget(parent) {
    video_widget_ = new QVideoWidget(this);
    player_ = new QMediaPlayer(this);
    auto audio_output = new QAudioOutput(this);
    player_->setVideoOutput(video_widget_);
    player_->setAudioOutput(audio_output);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(video_widget_);

    connect(player_, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        emit position_changed(pos / 1000.0);
    });

    connect(player_, &QMediaPlayer::durationChanged, this, [this](qint64 dur) {
        emit duration_changed(dur / 1000.0);
    });

    connect(player_, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit playback_finished();
        }
    });
}

void VideoPlayerWidget::load_video(const QString& file_path) {
    player_->setSource(QUrl::fromLocalFile(file_path));
}

void VideoPlayerWidget::play() {
    player_->play();
}

void VideoPlayerWidget::pause() {
    player_->pause();
}

void VideoPlayerWidget::stop() {
    player_->stop();
}

void VideoPlayerWidget::set_position(double seconds) {
    player_->setPosition(static_cast<qint64>(seconds * 1000));
}

double VideoPlayerWidget::get_position() const {
    return player_->position() / 1000.0;
}

double VideoPlayerWidget::get_duration() const {
    return player_->duration() / 1000.0;
}