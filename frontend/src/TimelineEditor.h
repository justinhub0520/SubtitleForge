#pragma once

#include <QWidget>
#include <QVector>
#include <QMouseEvent>

namespace subforge {
    struct Subtitle;
}

class TimelineEditor : public QWidget {
    Q_OBJECT

public:
    explicit TimelineEditor(QWidget *parent = nullptr);

    void set_subtitles(const QVector<subforge::Subtitle>& subtitles);
    QVector<subforge::Subtitle> get_subtitles() const;
    void set_current_time(double seconds);
    void set_duration(double duration);
    void set_zoom(double zoom);

signals:
    void subtitle_selected(int id);
    void subtitle_changed(const QVector<subforge::Subtitle>& subtitles);
    void time_requested(double seconds);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void draw_timeline(QPainter &painter);
    void draw_time_ruler(QPainter &painter);
    void draw_playhead(QPainter &painter);
    int time_to_x(double seconds) const;
    double x_to_time(int x) const;
    int find_subtitle_at(int x) const;

    QVector<subforge::Subtitle> subtitles_;
    double current_time_ = 0.0;
    double duration_ = 0.0;
    double zoom_ = 100.0;
    int selected_subtitle_id_ = -1;
    bool is_dragging_ = false;
    bool is_resizing_ = false;
    int drag_start_x_ = 0;
    int drag_subtitle_id_ = -1;
};