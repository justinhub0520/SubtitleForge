#include "TimelineEditor.h"
#include "subtitle.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

TimelineEditor::TimelineEditor(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(120);
    setMouseTracking(true);
}

void TimelineEditor::set_subtitles(const QVector<subforge::Subtitle>& subtitles) {
    subtitles_ = subtitles;
    update();
}

QVector<subforge::Subtitle> TimelineEditor::get_subtitles() const {
    return subtitles_;
}

void TimelineEditor::set_current_time(double seconds) {
    current_time_ = seconds;
    update();
}

void TimelineEditor::set_duration(double duration) {
    duration_ = duration;
    update();
}

void TimelineEditor::set_zoom(double zoom) {
    zoom_ = zoom;
    update();
}

int TimelineEditor::time_to_x(double seconds) const {
    return static_cast<int>(seconds * zoom_);
}

double TimelineEditor::x_to_time(int x) const {
    return x / zoom_;
}

int TimelineEditor::find_subtitle_at(int x) const {
    double time = x_to_time(x);
    for (int i = 0; i < subtitles_.size(); ++i) {
        if (time >= subtitles_[i].start_time && time <= subtitles_[i].end_time) {
            return i;
        }
    }
    return -1;
}

void TimelineEditor::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    draw_timeline(painter);
    draw_time_ruler(painter);
    draw_playhead(painter);
}

void TimelineEditor::draw_timeline(QPainter &painter) {
    painter.fillRect(rect(), QColor(30, 30, 30));

    int y_start = 30;
    int row_height = 60;

    for (int i = 0; i < subtitles_.size(); ++i) {
        const auto& sub = subtitles_[i];
        int x1 = time_to_x(sub.start_time);
        int x2 = time_to_x(sub.end_time);
        int width = x2 - x1;

        QColor color = (i == selected_subtitle_id_)
            ? QColor(66, 153, 225)
            : QColor(72, 72, 72);
        painter.fillRect(x1, y_start, width, row_height, color);

        painter.setPen(QColor(200, 200, 200));
        QFont font;
        font.setPointSize(9);
        painter.setFont(font);
        painter.drawText(x1 + 4, y_start + 20, QString::fromStdString(sub.text).left(30));
    }
}

void TimelineEditor::draw_time_ruler(QPainter &painter) {
    painter.fillRect(0, 0, width(), 25, QColor(40, 40, 40));
    painter.setPen(QColor(150, 150, 150));

    double step = 1.0;
    if (zoom_ < 20) step = 10.0;
    else if (zoom_ < 50) step = 5.0;
    else if (zoom_ < 200) step = 1.0;
    else step = 0.5;

    for (double t = 0; t < duration_; t += step) {
        int x = time_to_x(t);
        if (x > width()) break;
        painter.drawLine(x, 20, x, 25);

        int minutes = static_cast<int>(t) / 60;
        int seconds = static_cast<int>(t) % 60;
        painter.drawText(x + 2, 15,
            QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
    }
}

void TimelineEditor::draw_playhead(QPainter &painter) {
    int x = time_to_x(current_time_);
    painter.setPen(QPen(QColor(255, 0, 0), 2));
    painter.drawLine(x, 0, x, height());
}

void TimelineEditor::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int idx = find_subtitle_at(event->pos().x());
        if (idx >= 0) {
            selected_subtitle_id_ = idx;
            is_dragging_ = true;
            drag_start_x_ = event->pos().x();
            drag_subtitle_id_ = idx;
            emit subtitle_selected(idx);
        } else {
            emit time_requested(x_to_time(event->pos().x()));
        }
        update();
    }
}

void TimelineEditor::mouseMoveEvent(QMouseEvent *event) {
    if (is_dragging_ && drag_subtitle_id_ >= 0 && drag_subtitle_id_ < subtitles_.size()) {
        double delta = x_to_time(event->pos().x() - drag_start_x_);
        auto& sub = subtitles_[drag_subtitle_id_];
        sub.start_time = std::max(0.0, sub.start_time + delta);
        sub.end_time = std::max(sub.start_time + 0.1, sub.end_time + delta);
        drag_start_x_ = event->pos().x();
        emit subtitle_changed(subtitles_);
        update();
    }
}

void TimelineEditor::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        is_dragging_ = false;
        drag_subtitle_id_ = -1;
    }
}

void TimelineEditor::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        zoom_ = std::min(500.0, zoom_ * 1.2);
    } else {
        zoom_ = std::max(10.0, zoom_ / 1.2);
    }
    emit subtitle_changed(subtitles_);
    update();
}