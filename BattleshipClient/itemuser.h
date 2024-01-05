#ifndef ITEMUSER_H
#define ITEMUSER_H

#include <QWidget>

#include "utils.h"

namespace Ui { class ItemUser; }

class ItemUser : public QWidget {
    Q_OBJECT

public:
    explicit ItemUser(QWidget* parent = nullptr);
    ~ItemUser();

    void setData(const User& user);
    User getData() const;

signals:
    void onChallengeClick(User user);

private:
    Ui::ItemUser* ui;

    User user;
};

#endif // !ITEMUSER_H
