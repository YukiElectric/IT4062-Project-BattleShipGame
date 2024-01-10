#include "replaymanager.h"

ReplayManager::ReplayManager() {
    current = -1;
    count = 0;
    shotAnimation = new Blast(0, 0);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    timer->start(20);
}

ReplayManager::~ReplayManager() {

}

void ReplayManager::init(Match match, QList<Move> moves) {
    this->match = match;
    this->moves = moves;
    current = -1;
    count = 0;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            player1Board[r][c] = new BoardCell(player1Scene, r, c, RECT_SIZE);
            player1Board[r][c]->type = match.init1[r][c];

            player2Board[r][c] = new BoardCell(player2Scene, r, c, RECT_SIZE);
            player2Board[r][c]->type = match.init2[r][c];
        }
    }
}

void ReplayManager::next() {
    if (current == moves.size() - 1) {
        return;
    }

    current++;
    count = 30;
    shotAnimation->blastframe = 0;
    shotAnimation->incre = true;
    shotAnimation->setPos(moves[current].col * RECT_SIZE + (RECT_SIZE / 2), moves[current].row * RECT_SIZE + (RECT_SIZE / 2));
    if (moves[current].player == 0) {
        player2Scene->addItem(shotAnimation);
        emit onSetStatus(QString(match.player1.username) + "'s shot on (" + QString::number(moves[current].row) + ", " + QString::number(moves[current].col) + ")");
    } else {
        player1Scene->addItem(shotAnimation);
        emit onSetStatus(QString(match.player2.username) + "'s shot on (" + QString::number(moves[current].row) + ", " + QString::number(moves[current].col) + ")");
    }
}

void ReplayManager::prev() {
    if (current == -1) {
        return;
    }

    if (moves[current].player == 0) {
        if (moves[current].type == CELL_HIT) {
            player2Board[moves[current].row][moves[current].col]->type = CELL_SHIP;
        } else {
            player2Board[moves[current].row][moves[current].col]->type = CELL_EMPTY;
        }
    } else {
        if (moves[current].type == CELL_HIT) {
            player1Board[moves[current].row][moves[current].col]->type = CELL_SHIP;
        } else {
            player1Board[moves[current].row][moves[current].col]->type = CELL_EMPTY;
        }
    }

    player1Scene->update();
    player2Scene->update();

    current--;

    if (current > -1) {
        if (moves[current].player == 0) {
            emit onSetStatus(QString(match.player1.username) + "'s shot on (" + QString::number(moves[current].row) + ", " + QString::number(moves[current].col) + ")");
        } else {
            emit onSetStatus(QString(match.player2.username) + "'s shot on (" + QString::number(moves[current].row) + ", " + QString::number(moves[current].col) + ")");
        }
    } else {
        emit onSetStatus("Ready!!!");
    }
    emit onShotDone(current, moves.size());
}

void ReplayManager::advance() {
    if (count > 0) {
        --count;
        shotAnimation->advance();

        if (moves[current].player == 0) {
            if (count == 0) {
                player2Scene->removeItem(shotAnimation);
                player2Scene->update();
                player2Board[moves[current].row][moves[current].col]->type = moves[current].type;

                emit onShotDone(current, moves.size());
            }
        } else {
            if (count == 0) {
                player1Scene->removeItem(shotAnimation);
                player1Scene->update();
                player1Board[moves[current].row][moves[current].col]->type = moves[current].type;

                emit onShotDone(current, moves.size());
            }
        }
    }
}
