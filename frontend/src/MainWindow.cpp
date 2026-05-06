#include "MainWindow.h"
#include "VideoPlayerWidget.h"
#include "TimelineEditor.h"
#include "SubtitleListWidget.h"
#include "ApiClient.h"
#include "StyleEditorWidget.h"
#include "subtitle.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QProgressDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("SubForge - Video Subtitle Tool");
    resize(1200, 800);

    setup_ui();
    setup_menu();
    setup_toolbar();
    connect_signals();

    QSettings settings;
    api_client_->set_base_url(settings.value("server/url", "http://localhost:8080").toString());
}

void MainWindow::setup_ui() {
    auto central = new QWidget(this);
    setCentralWidget(central);

    auto main_layout = new QVBoxLayout(central);
    main_layout->setContentsMargins(4, 4, 4, 4);

    auto top_splitter = new QSplitter(Qt::Horizontal, this);
    video_player_ = new VideoPlayerWidget(this);
    top_splitter->addWidget(video_player_);

    style_editor_ = new StyleEditorWidget(this);
    style_editor_->setMaximumWidth(250);
    top_splitter->addWidget(style_editor_);

    top_splitter->setStretchFactor(0, 4);
    top_splitter->setStretchFactor(1, 1);
    main_layout->addWidget(top_splitter, 3);

    timeline_ = new TimelineEditor(this);
    main_layout->addWidget(timeline_, 1);

    subtitle_list_ = new SubtitleListWidget(this);
    main_layout->addWidget(subtitle_list_, 2);
}

void MainWindow::setup_menu() {
    auto file_menu = menuBar()->addMenu("&File");

    auto open_action = file_menu->addAction("&Open Video");
    open_action->setShortcut(QKeySequence("Ctrl+O"));
    connect(open_action, &QAction::triggered, this, &MainWindow::on_open_video);

    file_menu->addSeparator();

    auto exit_action = file_menu->addAction("E&xit");
    exit_action->setShortcut(QKeySequence("Ctrl+Q"));
    connect(exit_action, &QAction::triggered, this, &QMainWindow::close);

    auto tools_menu = menuBar()->addMenu("&Tools");

    auto generate_action = tools_menu->addAction("&Generate Subtitles");
    generate_action->setShortcut(QKeySequence("Ctrl+G"));
    connect(generate_action, &QAction::triggered, this, &MainWindow::on_generate_subtitles);

    auto export_menu = menuBar()->addMenu("&Export");

    auto export_video_action = export_menu->addAction("Export &Video with Subtitles");
    export_video_action->setShortcut(QKeySequence("Ctrl+E"));
    connect(export_video_action, &QAction::triggered, this, &MainWindow::on_export_video);

    auto export_srt_action = export_menu->addAction("Export &SRT");
    connect(export_srt_action, &QAction::triggered, this, &MainWindow::on_export_srt);
}

void MainWindow::setup_toolbar() {
    auto toolbar = addToolBar("Main Toolbar");

    auto open_action = toolbar->addAction("Open");
    connect(open_action, &QAction::triggered, this, &MainWindow::on_open_video);

    toolbar->addSeparator();

    auto play_action = toolbar->addAction("Play");
    connect(play_action, &QAction::triggered, video_player_, &VideoPlayerWidget::play);

    auto pause_action = toolbar->addAction("Pause");
    connect(pause_action, &QAction::triggered, video_player_, &VideoPlayerWidget::pause);

    auto stop_action = toolbar->addAction("Stop");
    connect(stop_action, &QAction::triggered, video_player_, &VideoPlayerWidget::stop);

    toolbar->addSeparator();

    auto generate_action = toolbar->addAction("Generate Subtitles");
    connect(generate_action, &QAction::triggered, this, &MainWindow::on_generate_subtitles);
}

void MainWindow::connect_signals() {
    connect(timeline_, &TimelineEditor::subtitle_selected, this, &MainWindow::on_subtitle_selected);
    connect(timeline_, &TimelineEditor::subtitle_changed, this, &MainWindow::on_subtitle_changed);
    connect(timeline_, &TimelineEditor::time_requested, this, &MainWindow::on_time_requested);

    connect(subtitle_list_, &SubtitleListWidget::subtitle_selected, this, &MainWindow::on_subtitle_selected);

    connect(video_player_, &VideoPlayerWidget::position_changed, this, &MainWindow::on_position_changed);
    connect(video_player_, &VideoPlayerWidget::duration_changed, this, &MainWindow::on_duration_changed);

    api_client_ = new ApiClient(this);

    connect(api_client_, &ApiClient::upload_finished, this, &MainWindow::on_upload_finished);
    connect(api_client_, &ApiClient::subtitles_ready, this, &MainWindow::on_subtitles_ready);
    connect(api_client_, &ApiClient::task_progress, this, &MainWindow::on_task_progress);
    connect(api_client_, &ApiClient::export_finished, this, &MainWindow::on_export_finished);
    connect(api_client_, &ApiClient::download_ready, this, &MainWindow::on_download_ready);
    connect(api_client_, &ApiClient::error, this, &MainWindow::on_api_error);
}

void MainWindow::on_open_video() {
    QString file_path = QFileDialog::getOpenFileName(this, "Open Video", "",
        "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv);;All Files (*)");
    if (file_path.isEmpty()) return;

    current_video_path_ = file_path;
    video_player_->load_video(file_path);
    update_status_bar("Video loaded: " + QFileInfo(file_path).fileName());

    api_client_->upload_video(file_path);
}

void MainWindow::on_generate_subtitles() {
    if (current_video_id_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please open and upload a video first.");
        return;
    }

    api_client_->generate_subtitles(current_video_id_);
    update_status_bar("Generating subtitles...");
}

void MainWindow::on_export_video() {
    if (current_video_id_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please open a video first.");
        return;
    }

    if (current_subtitles_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No subtitles to export.");
        return;
    }

    api_client_->export_video(current_video_id_, current_subtitles_);
    update_status_bar("Exporting video with subtitles...");
}

void MainWindow::on_export_srt() {
    if (current_subtitles_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No subtitles to export.");
        return;
    }

    QString file_path = QFileDialog::getSaveFileName(this, "Export SRT", "",
        "SRT Files (*.srt);;All Files (*)");
    if (file_path.isEmpty()) return;

    QFile file(file_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < current_subtitles_.size(); ++i) {
            const auto& sub = current_subtitles_[i];
            auto format_time = [](double seconds) -> QString {
                int hours = static_cast<int>(seconds) / 3600;
                int minutes = (static_cast<int>(seconds) % 3600) / 60;
                int secs = static_cast<int>(seconds) % 60;
                int ms = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);
                return QString("%1:%2:%3,%4")
                    .arg(hours, 2, 10, QChar('0'))
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(secs, 2, 10, QChar('0'))
                    .arg(ms, 3, 10, QChar('0'));
            };

            out << (i + 1) << "\n";
            out << format_time(sub.start_time) << " --> " << format_time(sub.end_time) << "\n";
            out << QString::fromStdString(sub.text) << "\n\n";
        }
        file.close();
        update_status_bar("SRT exported: " + QFileInfo(file_path).fileName());
    }
}

void MainWindow::on_subtitle_selected(int id) {
    for (const auto& sub : current_subtitles_) {
        if (sub.id == id) {
            video_player_->set_position(sub.start_time);
            subtitle_list_->select_subtitle(id);
            break;
        }
    }
}

void MainWindow::on_subtitle_changed(const QVector<subforge::Subtitle>& subtitles) {
    current_subtitles_ = subtitles;
    subtitle_list_->set_subtitles(subtitles);
}

void MainWindow::on_time_requested(double seconds) {
    video_player_->set_position(seconds);
}

void MainWindow::on_position_changed(double seconds) {
    timeline_->set_current_time(seconds);
}

void MainWindow::on_duration_changed(double seconds) {
    video_duration_ = seconds;
    timeline_->set_duration(seconds);
}

void MainWindow::on_upload_finished(const QString& video_id, double duration, int width, int height) {
    current_video_id_ = video_id;
    update_status_bar(QString("Video uploaded: %1 (%2s, %3x%4)")
        .arg(video_id).arg(duration, 0, 'f', 1).arg(width).arg(height));
}

void MainWindow::on_subtitles_ready(const QVector<subforge::Subtitle>& subtitles) {
    current_subtitles_ = subtitles;
    timeline_->set_subtitles(subtitles);
    subtitle_list_->set_subtitles(subtitles);
    update_status_bar(QString("Subtitles generated: %1 segments").arg(subtitles.size()));
}

void MainWindow::on_task_progress(const QString& task_id, int progress, const QString& status) {
    update_status_bar(QString("Task %1: %2% (%3)").arg(task_id).arg(progress).arg(status));
}

void MainWindow::on_export_finished(const QString& file_path) {
    update_status_bar("Export completed: " + file_path);
    api_client_->download_exported(current_video_id_);
}

void MainWindow::on_download_ready(const QByteArray& data, const QString& filename) {
    QString save_path = QFileDialog::getSaveFileName(this, "Save Exported Video",
        QFileInfo(current_video_path_).absolutePath() + "/subtitled_" + QFileInfo(current_video_path_).fileName(),
        "Video Files (*.mp4);;All Files (*)");
    if (save_path.isEmpty()) return;

    QFile file(save_path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        update_status_bar("Video saved: " + QFileInfo(save_path).fileName());
    }
}

void MainWindow::on_api_error(const QString& message) {
    QMessageBox::warning(this, "API Error", message);
    update_status_bar("Error: " + message);
}

void MainWindow::update_status_bar(const QString& message) {
    statusBar()->showMessage(message, 5000);
}