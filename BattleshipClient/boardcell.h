#ifndef BOARDCELL_H
#define BOARDCELL_H

#include <QObject>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include "utils.h"

class BoardCell : public QObject, public QGraphicsItem {
    Q_OBJECT

public:
    explicit BoardCell(QGraphicsScene* mscene, int r, int c, int size);

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    void onReceiveShot(CellType t);

    QGraphicsScene* scene;
    int r, c, size;
    CellType type;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

signals:
    void clicked(int r, int c);
};

#endif // BOARDCELL_H
