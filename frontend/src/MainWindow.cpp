#include "MainWindow.h"
#include "VideoPlayerWidget.h"
#include "TimelineEditor.h"
#include "SubtitleListWidget.h"
#include "StyleEditorWidget.h"
#include "ApiClient.h"
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
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("SubForge - Video Subtitle Tool");
    resize(1200, 800);

    api_client_ = new ApiClient(this);

    setup_ui();
    setup_menu();
    setup_toolbar();
    connect_signals();

    QSettings settings("SubForge", "SubForge");
    QString backend_url = settings.value("backend_url", "http://192.168.199.132:8080").toString();
    api_client_->set_base_url(backend_url);
}

void MainWindow::setup_ui() {
    auto central = new QWidget(this);
    setCentralWidget(central);

    auto main_layout = new QVBoxLayout(central);
    main_layout->setContentsMargins(4, 4, 4, 4);

    video_player_ = new VideoPlayerWidget(this);
    video_player_->setMinimumHeight(300);
    main_layout->addWidget(video_player_, 3);

    timeline_ = new TimelineEditor(this);
    timeline_->setMinimumHeight(100);
    main_layout->addWidget(timeline_, 1);

    auto bottom_splitter = new QSplitter(Qt::Horizontal, this);
    subtitle_list_ = new SubtitleListWidget(this);
    bottom_splitter->addWidget(subtitle_list_);

    style_editor_ = new StyleEditorWidget(this);
    bottom_splitter->addWidget(style_editor_);

    bottom_splitter->setSizes({700, 300});
    main_layout->addWidget(bottom_splitter, 2);
}

void MainWindow::setup_menu() {
    auto file_menu = menuBar()->addMenu("&File");

    auto open_action = file_menu->addAction("&Open Video (Local)");
    open_action->setShortcut(QKeySequence("Ctrl+O"));
    connect(open_action, &QAction::triggered, this, &MainWindow::on_open_video);

    auto upload_action = file_menu->addAction("&Upload Video to Server");
    upload_action->setShortcut(QKeySequence("Ctrl+U"));
    connect(upload_action, &QAction::triggered, this, &MainWindow::on_upload_video);

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

    auto upload_action = toolbar->addAction("Upload");
    connect(upload_action, &QAction::triggered, this, &MainWindow::on_upload_video);

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

    connect(style_editor_, &StyleEditorWidget::style_changed, this, &MainWindow::on_style_changed);

    connect(api_client_, &ApiClient::upload_finished, this, [this](const QString& video_id, double duration, int width, int height) {
        current_video_id_ = video_id;
        update_status_bar("Video uploaded: " + video_id + QString(" (%1x%2, %3s)").arg(width).arg(height).arg(duration, 0, 'f', 1));
    });

    connect(api_client_, &ApiClient::subtitles_ready, this, [this](const QList<subforge::Subtitle>& subtitles) {
        current_subtitles_ = subtitles;
        timeline_->set_subtitles(subtitles);
        subtitle_list_->set_subtitles(subtitles);
        update_status_bar("Subtitles generated: " + QString::number(subtitles.size()) + " segments");
    });

    connect(api_client_, &ApiClient::task_progress, this, [this](const QString& task_id, int progress, const QString& status) {
        update_status_bar("Task " + task_id + ": " + status + " (" + QString::number(progress) + "%)");
    });

    connect(api_client_, &ApiClient::error, this, [this](const QString& message) {
        QMessageBox::warning(this, "API Error", message);
    });

    connect(api_client_, &ApiClient::export_downloaded, this, [this](const QString& file_path) {
        update_status_bar("Video exported: " + file_path);
    });
}

void MainWindow::on_open_video() {
    QString file_path = QFileDialog::getOpenFileName(this, "Open Video", "",
        "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv);;All Files (*)");
    if (file_path.isEmpty()) return;

    current_video_path_ = file_path;
    video_player_->load_video(file_path);
    update_status_bar("Video loaded: " + QFileInfo(file_path).fileName());
}

void MainWindow::on_upload_video() {
    if (current_video_path_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please open a video first.");
        return;
    }

    update_status_bar("Uploading video...");
    api_client_->upload_video(current_video_path_);
}

void MainWindow::on_generate_subtitles() {
    if (current_video_id_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please upload a video to server first.");
        return;
    }

    update_status_bar("Generating subtitles...");
    api_client_->generate_subtitles(current_video_id_);
}

void MainWindow::on_export_video() {
    if (current_video_id_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please upload a video first.");
        return;
    }

    if (current_subtitles_.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No subtitles to export.");
        return;
    }

    QString default_dir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (default_dir.isEmpty()) {
        default_dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    if (default_dir.isEmpty()) {
        default_dir = QDir::homePath();
    }
    QString save_path = QFileDialog::getSaveFileName(this, "Export Video with Subtitles",
        default_dir + "/subtitled_video.mp4",
        "MP4 Video (*.mp4);;All Files (*)");
    if (save_path.isEmpty()) return;

    update_status_bar("Exporting video with subtitles...");
    api_client_->export_and_download(current_video_id_, save_path);
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
        update_status_bar("SRT exported: " + file_path);
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

void MainWindow::on_subtitle_changed(const QList<subforge::Subtitle>& subtitles) {
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

void MainWindow::on_style_changed(const subforge::SubtitleStyle& style) {
    for (auto& sub : current_subtitles_) {
        sub.style = style;
    }
    timeline_->set_subtitles(current_subtitles_);
    update_status_bar("Subtitle style updated");
}

void MainWindow::update_status_bar(const QString& message) {
    statusBar()->showMessage(message, 5000);
}
