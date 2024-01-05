#include "blast.h"

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>

Blast::Blast(qreal x, qreal y) {
    blastframe = 0;
    incre = true;

    int startX = 0;
    int startY = 0;

    startX = x;
    startY = y;

    setPos(mapToParent(startX, startY));
}

QRectF Blast::boundingRect() const {
    return QRect((-blastframe), (-blastframe), blastframe * 2, blastframe * 2);
}

void Blast::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QRectF rec = boundingRect();
    QPen pen(Qt::red);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawEllipse(rec);
}

void Blast::advance() {
    advance(1);
}

void Blast::advance(int phase) {
    if (!phase) {
        return;
    }

    if (incre) {
        ++blastframe;
        if (blastframe == 15) {
            incre = false;
        }
    } else {
        --blastframe;
        if (blastframe == 0) {
            incre = true;
        }
    }

    update();
}
