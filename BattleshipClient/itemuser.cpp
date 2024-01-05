#include "itemuser.h"
#include "ui_itemuser.h"

ItemUser::ItemUser(QWidget* parent) : QWidget(parent), ui(new Ui::ItemUser) {
    ui->setupUi(this);

    connect(ui->btnChallenge, &QPushButton::clicked, this, [this]() {
        emit onChallengeClick(this->user);
    });
}

ItemUser::~ItemUser() {
    delete ui;
}

void ItemUser::setData(const User& user) {
    this->user = user;

    ui->txtUsername->setText(user.username);
    ui->txtElo->setText(QString::number(user.elo));
}

User ItemUser::getData() const {
    return this->user;
}
