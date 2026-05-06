#pragma once

#include <QWidget>
#include "subtitle.h"

class QFontComboBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;

class StyleEditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit StyleEditorWidget(QWidget *parent = nullptr);

    void set_style(const subforge::SubtitleStyle& style);
    subforge::SubtitleStyle get_style() const;

signals:
    void style_changed(const subforge::SubtitleStyle& style);

private slots:
    void on_style_changed();

private:
    void setup_ui();

    QFontComboBox* font_combo_;
    QSpinBox* font_size_spin_;
    QComboBox* position_combo_;
    QCheckBox* bold_check_;
    QCheckBox* italic_check_;
    QPushButton* color_button_;
    QPushButton* bg_color_button_;

    subforge::SubtitleStyle current_style_;
};