#ifndef REPLAYMANAGER_H
#define REPLAYMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include "utils.h"
#include "boardcell.h"
#include "blast.h"

class ReplayManager : public QObject {
    Q_OBJECT

public:
    ReplayManager();
    ~ReplayManager();

    void init(Match match, QList<Move> moves);
    void next();
    void prev();

    static const int RECT_SIZE = 30;

    QTimer* timer;
    QGraphicsScene* player1Scene;
    QGraphicsScene* player2Scene;
    BoardCell* player1Board[BOARD_SIZE][BOARD_SIZE];
    BoardCell* player2Board[BOARD_SIZE][BOARD_SIZE];
    Blast* shotAnimation;

    Match match;
    QList<Move> moves;
    int current, count;

signals:
    void onSetStatus(QString status);
    void onShotDone(int cur, int size);

private slots:
    void advance();
};

#endif // REPLAYMANAGER_H
