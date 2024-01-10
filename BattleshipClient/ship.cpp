#include "ship.h"

#include "gamemanager.h"

int Ship::x_array[] = {0, 0, 0, 0, 0};
int Ship::y_array[] = {0, 0, 0, 0, 0};
int Ship::size_array[] = {2, 3, 3, 4, 5};
bool Ship::orientation_array[] = {0, 0, 0, 0, 0};

Ship::Ship(QGraphicsScene* mscene, int id) {
    this->scene = mscene;
    this->id = id;
    size = size_array[id];
    orient = 0;
    pressed = false;
    orientation_array[id] = 0;
    setX(id * GameManager::RECT_SIZE);
    setY(0);
    x_array[id] = this->pos().x();
    y_array[id] = this->pos().y();
    setFlag(ItemIsMovable);
}

void Ship::reOrientShip() {
    QPointF xandy;
    int currentX, currentY;

    xandy = pos();
    currentX = xandy.x();
    currentY = xandy.y();

    if (orient == 0) {
        orient = 1;
        orientation_array[id] = 1;
        setX(currentX + 65);
        setY(currentY + 50);
        setRotation(90);
    } else {
        orient = 0;
        orientation_array[id] = 0;
        setX(currentX - 65);
        setY(currentY - 50);
        setRotation(0);
    }
    update();
}

QRectF Ship::boundingRect() const {
    switch(size) {
    case 2:
        return QRectF(0, 0, GameManager::RECT_SIZE, GameManager::RECT_SIZE * 2);

    case 3:
        return QRectF(0, 0, GameManager::RECT_SIZE, GameManager::RECT_SIZE * 3);

    case 4:
        return QRectF(0, 0, GameManager::RECT_SIZE, GameManager::RECT_SIZE * 4);

    case 5:
        return QRectF(0, 0, GameManager::RECT_SIZE, GameManager::RECT_SIZE * 5);
    }

    return QRectF();
}

void Ship::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QRectF rec = boundingRect();
    QBrush brush(Qt::blue);

    if(pressed) {
        brush.setColor(Qt::red);
    } else {
        brush.setColor(Qt::green);
    }

    painter->fillRect(rec, brush);
    painter->drawRect(rec);
}

void Ship::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}


void Ship::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    reOrientShip();
    QGraphicsItem::mouseDoubleClickEvent(event);
}


void Ship::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    pressed = false;

    QPointF xandy;
    int x, y, currentX, currentY;

    xandy = pos();
    currentX = xandy.x();
    currentY = xandy.y();

    if (currentY < 0) {
        setY(0);
    }

    if (currentX < 0) {
        x = 0;
    } else {
        x = (currentX / GameManager::RECT_SIZE * GameManager::RECT_SIZE);
        if ((currentX % GameManager::RECT_SIZE) >= (GameManager::RECT_SIZE / 2)) {
            x = x + GameManager::RECT_SIZE;
        }
    }

    if (currentY < 0) {
        y = 0;
    } else {
        y = (currentY / GameManager::RECT_SIZE * GameManager::RECT_SIZE);
        if ((currentY % GameManager::RECT_SIZE) >= (GameManager::RECT_SIZE / 2)) {
            y = y + GameManager::RECT_SIZE;
        }
    }

    if (orient == 0) {
        if (x > ((GameManager::RECT_SIZE * BOARD_SIZE) - GameManager::RECT_SIZE)) {
            x = (GameManager::RECT_SIZE * BOARD_SIZE) - GameManager::RECT_SIZE;
        }

        if (y > ((GameManager::RECT_SIZE * BOARD_SIZE)  - (GameManager::RECT_SIZE * size))) {
            y = (GameManager::RECT_SIZE * BOARD_SIZE) - (GameManager::RECT_SIZE * size);
        }
    } else {
        if (y > ((GameManager::RECT_SIZE * BOARD_SIZE) - GameManager::RECT_SIZE)) {
            y = (GameManager::RECT_SIZE * BOARD_SIZE) - GameManager::RECT_SIZE;
        }

        if (x > ((GameManager::RECT_SIZE * BOARD_SIZE))) {
            x = (GameManager::RECT_SIZE * BOARD_SIZE);
        }

        if (x < (GameManager::RECT_SIZE * size)) {
            x = (GameManager::RECT_SIZE * size);
        }
    }

    setX(x);
    setY(y);

    bool pass = false;
    while(pass == false) {
        pass = true;
    }

    x_array[id] = this->pos().x();
    y_array[id] = this->pos().y();

    update();
    QGraphicsItem::mouseReleaseEvent(event);
    emit onShipPlaced();
}


void Ship::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    x_array[id] = this->pos().x();
    y_array[id] = this->pos().y();
    update();
    QGraphicsItem::mouseMoveEvent(event);
}
