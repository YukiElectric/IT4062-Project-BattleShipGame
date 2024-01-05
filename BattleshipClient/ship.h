#ifndef SHIP_H
#define SHIP_H

#include <QPainter>
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>

class Ship : public QObject, public QGraphicsItem {
    Q_OBJECT

public:
    Ship(QGraphicsScene* mscene, int id);

    void reOrientShip();

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    static int x_array[5];
    static int y_array[5];
    static int size_array[5];
    static bool orientation_array[5];

    QGraphicsScene* scene;
    int id;
    int size;
    bool orient;
    bool pressed;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

signals:
    void onShipPlaced();
};

#endif // SHIP_H
