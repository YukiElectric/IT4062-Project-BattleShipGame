#include "itemhistory.h"
#include "ui_itemhistory.h"

ItemHistory::ItemHistory(QWidget* parent) : QWidget(parent), ui(new Ui::ItemHistory) {
    ui->setupUi(this);
}

ItemHistory::~ItemHistory() {
    delete ui;
}

void ItemHistory::mouseDoubleClickEvent(QMouseEvent* event) {
    emit onClick(match);
}

void ItemHistory::setData(QListWidgetItem& item, const User& current, const Match& match) {
    this->current = current;
    this->match = match;

    if (current == match.player1) {
        ui->txtUsername->setText(QString(match.player2.username));

        if (match.winner == 0) {
            item.setBackground(QColor("#C8E6C9"));
            ui->txtPoint->setStyleSheet("color: #4CAF50;");
            ui->txtPoint->setText("+" + QString::number(match.point1));
        } else {
            item.setBackground(QColor("#FFCDD2"));
            ui->txtPoint->setStyleSheet("color: #F44336;");
            ui->txtPoint->setText("-" + QString::number(match.point1));
        }
    } else {
        ui->txtUsername->setText(QString(match.player1.username));

        if (match.winner == 1) {
            item.setBackground(QColor("#C8E6C9"));
            ui->txtPoint->setStyleSheet("color: #4CAF50;");
            ui->txtPoint->setText("+" + QString::number(match.point2));
        } else {
            item.setBackground(QColor("#FFCDD2"));
            ui->txtPoint->setStyleSheet("color: #F44336;");
            ui->txtPoint->setText((match.point2 == 0 ? "-" : "") + QString::number(match.point2));
        }
    }
}

Match ItemHistory::getData() const {
    return this->match;
}
