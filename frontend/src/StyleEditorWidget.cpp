#include "StyleEditorWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFontComboBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>

StyleEditorWidget::StyleEditorWidget(QWidget *parent) : QWidget(parent) {
    setup_ui();
}

void StyleEditorWidget::setup_ui() {
    auto layout = new QFormLayout(this);

    font_combo_ = new QFontComboBox(this);
    layout->addRow("Font:", font_combo_);

    font_size_spin_ = new QSpinBox(this);
    font_size_spin_->setRange(8, 72);
    font_size_spin_->setValue(24);
    layout->addRow("Size:", font_size_spin_);

    position_combo_ = new QComboBox(this);
    position_combo_->addItems({"bottom_center", "top_center", "middle_center"});
    layout->addRow("Position:", position_combo_);

    bold_check_ = new QCheckBox("Bold", this);
    italic_check_ = new QCheckBox("Italic", this);
    auto style_layout = new QHBoxLayout();
    style_layout->addWidget(bold_check_);
    style_layout->addWidget(italic_check_);
    layout->addRow("Style:", style_layout);

    color_button_ = new QPushButton("Text Color", this);
    bg_color_button_ = new QPushButton("Background Color", this);
    auto color_layout = new QHBoxLayout();
    color_layout->addWidget(color_button_);
    color_layout->addWidget(bg_color_button_);
    layout->addRow("Colors:", color_layout);

    connect(font_combo_, &QFontComboBox::currentFontChanged, this, &StyleEditorWidget::on_style_changed);
    connect(font_size_spin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &StyleEditorWidget::on_style_changed);
    connect(position_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StyleEditorWidget::on_style_changed);
    connect(bold_check_, &QCheckBox::toggled, this, &StyleEditorWidget::on_style_changed);
    connect(italic_check_, &QCheckBox::toggled, this, &StyleEditorWidget::on_style_changed);
    connect(color_button_, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(QColor(QString::fromStdString(current_style_.color)), this);
        if (color.isValid()) {
            current_style_.color = color.name().toStdString();
            emit style_changed(current_style_);
        }
    });
    connect(bg_color_button_, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(QColor(QString::fromStdString(current_style_.bg_color)), this);
        if (color.isValid()) {
            current_style_.bg_color = color.name(QColor::HexArgb).toStdString();
            emit style_changed(current_style_);
        }
    });
}

void StyleEditorWidget::set_style(const subforge::SubtitleStyle& style) {
    current_style_ = style;
    font_combo_->setCurrentFont(QFont(QString::fromStdString(style.font_family)));
    font_size_spin_->setValue(style.font_size);
    bold_check_->setChecked(style.bold);
    italic_check_->setChecked(style.italic);

    int pos_idx = position_combo_->findText(QString::fromStdString(style.position));
    if (pos_idx >= 0) position_combo_->setCurrentIndex(pos_idx);
}

subforge::SubtitleStyle StyleEditorWidget::get_style() const {
    return current_style_;
}

void StyleEditorWidget::on_style_changed() {
    current_style_.font_family = font_combo_->currentFont().family().toStdString();
    current_style_.font_size = font_size_spin_->value();
    current_style_.bold = bold_check_->isChecked();
    current_style_.italic = italic_check_->isChecked();
    current_style_.position = position_combo_->currentText().toStdString();

    emit style_changed(current_style_);
}