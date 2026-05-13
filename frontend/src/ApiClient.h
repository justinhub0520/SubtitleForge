#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "subtitle.h"

class ApiClient : public QObject {
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);

    void set_base_url(const QString& url);

    void upload_video(const QString& file_path);
    void generate_subtitles(const QString& video_id, const QString& language = "auto");
    void get_task_status(const QString& task_id);
    void download_subtitles(const QString& video_id);
    void export_video(const QString& video_id, const QList<subforge::Subtitle>& subtitles);
    void export_and_download(const QString& video_id, const QString& save_path);
    void download_exported(const QString& video_id);

signals:
    void upload_finished(const QString& video_id, double duration, int width, int height);
    void subtitles_ready(const QList<subforge::Subtitle>& subtitles);
    void task_progress(const QString& task_id, int progress, const QString& status);
    void export_finished(const QString& file_path);
    void export_downloaded(const QString& file_path);
    void download_ready(const QByteArray& data, const QString& filename);
    void error(const QString& message);

private slots:
    void on_upload_finished(QNetworkReply* reply);
    void on_task_status_finished(QNetworkReply* reply);
    void on_subtitles_finished(QNetworkReply* reply);
    void on_export_finished(QNetworkReply* reply);
    void on_download_finished(QNetworkReply* reply);

private:
    QString base_url_;
    QNetworkAccessManager* network_manager_;
    QString pending_task_id_;
};