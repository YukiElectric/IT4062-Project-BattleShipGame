#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include "utils.h"
#include "boardcell.h"
#include "blast.h"
#include "ship.h"

class GameManager : public QObject {
    Q_OBJECT

public:
    GameManager();
    ~GameManager();

    void init();
    void clearShips();
    void myShotDone(CellType type);
    void enemyShot(int row, int col, CellType type);
    bool allShipsArePlaced();

    static const int RECT_SIZE = 30;

    QTimer* timer;

    QGraphicsScene* myScene;
    QGraphicsScene* enemyScene;
    BoardCell* myBoard[BOARD_SIZE][BOARD_SIZE];
    BoardCell* enemyBoard[BOARD_SIZE][BOARD_SIZE];
    Ship* ships[5];
    Blast* shotAnimation;

    int count, curRow, curCol;
    CellType type;
    bool myTurn, isShooting, isRunning;

signals:
    void onShot(int row, int col);
    void onShipPlaced();
    void onSetStatus(QString status);

private slots:
    void onCellClicked(int row, int col);
    void advance();
    void shipPlaced();
};

#endif // GAMEMANAGER_H
