#ifndef BLAST_H
#define BLAST_H

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>

class Blast : public QGraphicsItem {
public:
    Blast(qreal x, qreal y);
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);
    void advance();
    short int blastframe;
    bool incre;

protected:
    void advance(int phase);
};

#endif // !BLAST_H
