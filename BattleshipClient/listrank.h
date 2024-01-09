#ifndef LISTRANK_H
#define LISTRANK_H

#include <QMainWindow>

namespace Ui {
class listrank;
}

class listrank : public QMainWindow
{
    Q_OBJECT

public:
    explicit listrank(QWidget *parent = nullptr);
    ~listrank();

private:
    Ui::listrank *ui;
};

#endif // LISTRANK_H
