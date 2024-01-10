#ifndef ITEMHISTORY_H
#define ITEMHISTORY_H

#include <QWidget>
#include <QListWidgetItem>

#include "utils.h"

namespace Ui { class ItemHistory; }

class ItemHistory : public QWidget {
    Q_OBJECT

public:
    explicit ItemHistory(QWidget* parent = nullptr);
    ~ItemHistory();

    void setData(QListWidgetItem& item, const User& current, const Match& match);
    Match getData() const;

protected:
    void mouseDoubleClickEvent(QMouseEvent* event);

signals:
    void onClick(Match match);

private:
    Ui::ItemHistory* ui;

    Match match;
    User current;
};

#endif // ITEMHISTORY_H
