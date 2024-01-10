#include "boardcell.h"

BoardCell::BoardCell(QGraphicsScene* mscene, int r, int c, int size)
        : scene(mscene), r(r), c(c), size(size), type(CELL_EMPTY) {
    setFlag(ItemIsSelectable);
    scene->addItem(this);
}

QRectF BoardCell::boundingRect() const {
    return QRectF(c * size, r * size, size, size);
}

void BoardCell::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QRectF rec = boundingRect();
    QBrush brush(Qt::gray);
    if (type == CELL_MISS) {
        brush.setColor(Qt::blue);
    } else if (type == CELL_HIT) {
        brush.setColor(Qt::red);
    } else if (type == CELL_SHIP) {
        brush.setColor(Qt::green);
    } else {
        brush.setColor(Qt::gray);
    }
    painter->fillRect(rec, brush);
    painter->drawRect(rec);
}

void BoardCell::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    update();
    QGraphicsItem::mousePressEvent(event);
}


void BoardCell::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(r, c);

    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void BoardCell::onReceiveShot(CellType t) {
    setEnabled(false);
    type = t;
    update();
}
