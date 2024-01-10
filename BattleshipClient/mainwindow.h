#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

#include "utils.h"
#include "gamemanager.h"
#include "replaymanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onReadyRead();
    void onSocketDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);

    void setStatusText(QString status);
    void setReplayStatusText(QString status);
    void challengeClick(User user);
    void itemHistoryClick(Match match);
    void notifyChallengeRequest(User user);
    void updateListWidget(User user);
    void updateListWidgetHistory(Match math);
    void shipPlaced();
    void shot(int row, int col);
    void shotDone(int current, int size);
    void showReplayPage(Match match, QList<Move> moves);

    void getUsers();
    void getMatches();
    void getMoves(const Match& match);

    void handleResponse(QByteArray& bytes);
    void handleSignIn(const Response& response, QByteArray& bytes);
    void handleSignUp(const Response& response, QByteArray& bytes);
    void handleSignOut(const Response& response, QByteArray& bytes);
    void handleGetUsers(const Response& response, QByteArray& bytes);
    void handleGetMatches(const Response& response, QByteArray& bytes);
    void handleGetMoves(const Response& response, QByteArray& bytes);
    void handleOnline(const Response& response, QByteArray& bytes);
    void handleOffline(const Response& response, QByteArray& bytes);
    void handleChallenge(const Response& response, QByteArray& bytes);
    void handleAccept(const Response& response, QByteArray& bytes);
    void handleDecline(const Response& response, QByteArray& bytes);
    void handleReady(const Response& response, QByteArray& bytes);
    void handleShot(const Response& response, QByteArray& bytes);
    void handleGameOver(const Response& response, QByteArray& bytes);
    void handleQuit(const Response& response, QByteArray& bytes);
    void handleSurrender(const Response& response, QByteArray& bytes);
    void handleGetRank(const Response& response, QByteArray& bytes);
    void handleChat(const Response& response, QByteArray& bytes);
    void handleQuickMatch(const Response& response, QByteArray& bytes);

private:
    Ui::MainWindow* ui;

    QTcpSocket* socket;

    QByteArray buffer;
    User current;
    Match curMatch;

    GameManager* gameManager;
    ReplayManager* replayManager;
    bool quickmatch;
};

#endif // !MAINWINDOW_H
