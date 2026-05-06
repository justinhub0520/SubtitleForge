#pragma once

#include <QTableWidget>
#include <QVector>

namespace subforge {
    struct Subtitle;
}

class SubtitleListWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit SubtitleListWidget(QWidget *parent = nullptr);

    void set_subtitles(const QVector<subforge::Subtitle>& subtitles);
    QVector<subforge::Subtitle> get_subtitles() const;
    void select_subtitle(int id);

signals:
    void subtitle_selected(int id);
    void subtitle_edited(int id, const QString& text);

private slots:
    void on_cell_changed(int row, int column);
    void on_cell_clicked(int row, int column);

private:
    QVector<subforge::Subtitle> subtitles_;
    bool is_updating_ = false;
};