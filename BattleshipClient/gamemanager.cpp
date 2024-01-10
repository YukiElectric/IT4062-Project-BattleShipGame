#include "gamemanager.h"

#include <QPen>
#include <QBrush>

GameManager::GameManager() {
    myTurn = true;
    shotAnimation = new Blast(0, 0);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    timer->start(500 / 20);
}

GameManager::~GameManager() {

}

void GameManager::init() {
    isRunning = false;
    count = 0;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            myBoard[r][c] = new BoardCell(myScene, r, c, RECT_SIZE);
            enemyBoard[r][c] = new BoardCell(enemyScene, r, c, RECT_SIZE);
            connect(enemyBoard[r][c], &BoardCell::clicked, this, &GameManager::onCellClicked);
        }
    }

    for (int i = 0; i < 5; i++) {
        ships[i] = new Ship(myScene, i);
        myScene->addItem(ships[i]);
        connect(ships[i], &Ship::onShipPlaced, this, &GameManager::shipPlaced);
    }
}

void GameManager::clear() {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            delete myBoard[r][c];
            delete enemyBoard[r][c];
        }
    }
}

void GameManager::clearShips() {
    for (int i = 0; i < 5; i++) {
        myScene->removeItem(ships[i]);
        delete ships[i];
    }
}

void GameManager::onCellClicked(int row, int col) {
    qDebug() << "onCellClicked():" << row << col << count << myTurn;

    if (count > 0 || !myTurn) {
        return;
    }

    curRow = row;
    curCol = col;
    isShooting = true;
    count = 30;
    shotAnimation->blastframe = 0;
    shotAnimation->incre = true;
    shotAnimation->setPos(col * RECT_SIZE + (RECT_SIZE / 2), row * RECT_SIZE + (RECT_SIZE / 2));
    enemyScene->addItem(shotAnimation);

    emit onShot(row, col);
}

void GameManager::advance() {
    if (count > 0 && isRunning) {
        if (myTurn) {
            --count;
            shotAnimation->advance();
            if (count == 0) {
                if (isShooting) {
                    count = 30;
                    return;
                }

                myTurn = !myTurn;
                enemyScene->removeItem(shotAnimation);
                enemyScene->update();
                enemyBoard[curRow][curCol]->onReceiveShot(type);
                emit onSetStatus("Opponent's turn!");
            }
        } else {
            --count;
            shotAnimation->advance();
            if (count == 0) {
                myTurn = !myTurn;
                myScene->removeItem(shotAnimation);
                myScene->update();
                myBoard[curRow][curCol]->onReceiveShot(type);
                emit onSetStatus("Your turn!");
            }
        }
    }
}

void GameManager::myShotDone(CellType type) {
    this->type = type;
    isShooting = false;
}

void GameManager::enemyShot(int row, int col, CellType type) {
    this->type = type;
    curRow = row;
    curCol = col;
    myTurn = false;
    count = 30;
    shotAnimation->blastframe = 0;
    shotAnimation->incre = true;
    shotAnimation->setPos(col * RECT_SIZE + (RECT_SIZE / 2), row * RECT_SIZE + (RECT_SIZE / 2));
    myScene->addItem(shotAnimation);
}

bool GameManager::allShipsArePlaced() {
    QRectF* rectShips[5];

    if (Ship::orientation_array[0] == 0) {
        rectShips[0] = new QRectF(Ship::x_array[0], Ship::y_array[0], RECT_SIZE, RECT_SIZE * 2);
    } else {
        rectShips[0] = new QRectF(Ship::x_array[0] - RECT_SIZE * 2, Ship::y_array[0], RECT_SIZE * 2, RECT_SIZE);
    }
    if (Ship::orientation_array[1] == 0) {
        rectShips[1] = new QRectF(Ship::x_array[1], Ship::y_array[1], RECT_SIZE, RECT_SIZE * 3);
    } else {
        rectShips[1] = new QRectF(Ship::x_array[1] - RECT_SIZE * 3, Ship::y_array[1], RECT_SIZE * 3, RECT_SIZE);
    }
    if (Ship::orientation_array[2] == 0) {
        rectShips[2] = new QRectF(Ship::x_array[2], Ship::y_array[2], RECT_SIZE, RECT_SIZE * 3);
    } else {
        rectShips[2] = new QRectF(Ship::x_array[2]-RECT_SIZE * 3,Ship::y_array[2], RECT_SIZE * 3, RECT_SIZE);
    }
    if (Ship::orientation_array[3] == 0) {
        rectShips[3] = new QRectF(Ship::x_array[3],Ship::y_array[3], RECT_SIZE, RECT_SIZE * 4);
    } else {
        rectShips[3] = new QRectF(Ship::x_array[3] - RECT_SIZE * 4,Ship::y_array[3], RECT_SIZE * 4, RECT_SIZE);
    }
    if (Ship::orientation_array[4]==0) {
        rectShips[4] = new QRectF(Ship::x_array[4], Ship::y_array[4], RECT_SIZE,RECT_SIZE * 5);
    } else {
        rectShips[4] = new QRectF(Ship::x_array[4] - RECT_SIZE * 5, Ship::y_array[4], RECT_SIZE * 5, RECT_SIZE);
    }

    for (int first = 1; first < 5; ++first) {
        for (int second = 0;second < first; ++second) {
            if (rectShips[first]->intersects(*rectShips[second]) || rectShips[first]->contains(*rectShips[second])) {
                delete rectShips[0];
                delete rectShips[1];
                delete rectShips[2];
                delete rectShips[3];
                delete rectShips[4];
                return false;
            }
        }
    }

    delete rectShips[0];
    delete rectShips[1];
    delete rectShips[2];
    delete rectShips[3];
    delete rectShips[4];

    return true;
}

void GameManager::shipPlaced() {
    emit onShipPlaced();
}
