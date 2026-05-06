#include "SubtitleListWidget.h"
#include "subtitle.h"
#include <QHeaderView>

SubtitleListWidget::SubtitleListWidget(QWidget *parent) : QTableWidget(parent) {
    setColumnCount(4);
    setHorizontalHeaderLabels({"ID", "Start", "End", "Text"});
    horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::DoubleClicked);

    connect(this, &QTableWidget::cellChanged, this, &SubtitleListWidget::on_cell_changed);
    connect(this, &QTableWidget::cellClicked, this, &SubtitleListWidget::on_cell_clicked);
}

void SubtitleListWidget::set_subtitles(const QVector<subforge::Subtitle>& subtitles) {
    is_updating_ = true;
    subtitles_ = subtitles;
    setRowCount(subtitles.size());

    for (int i = 0; i < subtitles.size(); ++i) {
        const auto& sub = subtitles[i];
        setItem(i, 0, new QTableWidgetItem(QString::number(sub.id)));
        setItem(i, 1, new QTableWidgetItem(QString::number(sub.start_time, 'f', 2)));
        setItem(i, 2, new QTableWidgetItem(QString::number(sub.end_time, 'f', 2)));
        setItem(i, 3, new QTableWidgetItem(QString::fromStdString(sub.text)));

        item(i, 0)->setTextAlignment(Qt::AlignCenter);
        item(i, 1)->setTextAlignment(Qt::AlignCenter);
        item(i, 2)->setTextAlignment(Qt::AlignCenter);
    }

    is_updating_ = false;
}

QVector<subforge::Subtitle> SubtitleListWidget::get_subtitles() const {
    return subtitles_;
}

void SubtitleListWidget::select_subtitle(int id) {
    for (int i = 0; i < subtitles_.size(); ++i) {
        if (subtitles_[i].id == id) {
            selectRow(i);
            scrollToItem(item(i, 0));
            break;
        }
    }
}

void SubtitleListWidget::on_cell_changed(int row, int column) {
    if (is_updating_) return;

    if (row >= 0 && row < subtitles_.size()) {
        if (column == 3) {
            subtitles_[row].text = item(row, 3)->text().toStdString();
            emit subtitle_edited(row, item(row, 3)->text());
        }
    }
}

void SubtitleListWidget::on_cell_clicked(int row, int column) {
    if (row >= 0 && row < subtitles_.size()) {
        emit subtitle_selected(subtitles_[row].id);
    }
}