#include "ApiClient.h"
#include "subtitle.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

ApiClient::ApiClient(QObject *parent) : QObject(parent) {
    network_manager_ = new QNetworkAccessManager(this);
}

void ApiClient::set_base_url(const QString& url) {
    base_url_ = url;
}

void ApiClient::upload_video(const QString& file_path) {
    QUrl url(base_url_ + "/api/videos/upload");
    QNetworkRequest request(url);

    QFile *file = new QFile(file_path);
    if (!file->open(QIODevice::ReadOnly)) {
        emit error("Cannot open file: " + file_path);
        file->deleteLater();
        return;
    }

    QHttpMultiPart *multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart video_part;
    video_part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("video/mp4"));
    video_part.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant("form-data; name=\"video\"; filename=\"" + QFileInfo(file_path).fileName() + "\""));
    video_part.setBodyDevice(file);
    file->setParent(multi_part);
    multi_part->append(video_part);

    QNetworkReply *reply = network_manager_->post(request, multi_part);
    multi_part->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        on_upload_finished(reply);
    });
}

void ApiClient::generate_subtitles(const QString& video_id, const QString& language) {
    QUrl url(base_url_ + "/api/tasks/transcribe");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["video_id"] = video_id;
    body["language"] = language;

    QNetworkReply *reply = network_manager_->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        pending_task_id_ = obj["task_id"].toString();

        emit task_progress(pending_task_id_, 0, "processing");

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            get_task_status(pending_task_id_);
            if (!pending_task_id_.isEmpty()) {
                timer->start(2000);
            } else {
                timer->stop();
                timer->deleteLater();
            }
        });
        timer->start(2000);

        reply->deleteLater();
    });
}

void ApiClient::get_task_status(const QString& task_id) {
    QUrl url(base_url_ + "/api/tasks/" + task_id);
    QNetworkRequest request(url);

    QNetworkReply *reply = network_manager_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, task_id]() {
        on_task_status_finished(reply);
    });
}

void ApiClient::download_subtitles(const QString& video_id) {
    QUrl url(base_url_ + "/api/videos/" + video_id + "/subtitles");
    QNetworkRequest request(url);

    QNetworkReply *reply = network_manager_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, video_id]() {
        on_subtitles_finished(reply);
    });
}

void ApiClient::export_video(const QString& video_id, const QVector<subforge::Subtitle>& subtitles) {
    QUrl url(base_url_ + "/api/tasks/export");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["video_id"] = video_id;

    QJsonArray subs_array;
    for (const auto& sub : subtitles) {
        QJsonObject sub_obj;
        sub_obj["id"] = sub.id;
        sub_obj["start_time"] = sub.start_time;
        sub_obj["end_time"] = sub.end_time;
        sub_obj["text"] = QString::fromStdString(sub.text);
        subs_array.append(sub_obj);
    }
    body["subtitles"] = subs_array;

    QNetworkReply *reply = network_manager_->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        on_export_finished(reply);
    });
}

void ApiClient::download_exported(const QString& video_id) {
    QUrl url(base_url_ + "/api/videos/" + video_id + "/download");
    QNetworkRequest request(url);

    QNetworkReply *reply = network_manager_->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, video_id]() {
        on_download_finished(reply);
    });
}

void ApiClient::on_upload_finished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error("Upload failed: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QString video_id = obj["video_id"].toString();
    double duration = obj["duration"].toDouble();
    int width = obj["width"].toInt();
    int height = obj["height"].toInt();

    emit upload_finished(video_id, duration, width, height);
    reply->deleteLater();
}

void ApiClient::on_task_status_finished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QString task_id = obj["task_id"].toString();
    int progress = obj["progress"].toInt();
    QString status = obj["status"].toString();

    emit task_progress(task_id, progress, status);

    if (status == "done") {
        pending_task_id_.clear();
        QString video_id = obj["video_id"].toString();
        download_subtitles(video_id);
    } else if (status == "error") {
        pending_task_id_.clear();
        emit error("Transcription failed: " + obj["error_message"].toString());
    }

    reply->deleteLater();
}

void ApiClient::on_subtitles_finished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error("Failed to download subtitles: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray arr = doc.array();

    QVector<subforge::Subtitle> subtitles;
    for (const auto& val : arr) {
        QJsonObject obj = val.toObject();
        subforge::Subtitle sub;
        sub.id = obj["id"].toInt();
        sub.start_time = obj["start_time"].toDouble();
        sub.end_time = obj["end_time"].toDouble();
        sub.text = obj["text"].toString().toStdString();
        subtitles.append(sub);
    }

    emit subtitles_ready(subtitles);
    reply->deleteLater();
}

void ApiClient::on_export_finished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error("Export failed: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    emit export_finished(obj["file_path"].toString());
    reply->deleteLater();
}

void ApiClient::on_download_finished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error("Download failed: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QString filename = reply->rawHeader("Content-Disposition");
    emit download_ready(data, filename);
    reply->deleteLater();
}