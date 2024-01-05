#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

#include "utils.h"
#include "itemuser.h"
#include "gamemanager.h"

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
    void challengeClick(User user);
    void notifyChallengeRequest(User user);
    void updateListWidget(User user);
    void shipPlaced();
    void shot(int row, int col);

    void getUsers();

    void handleResponse(QByteArray& bytes);
    void handleSignIn(const Response& response, QByteArray& bytes);
    void handleSignUp(const Response& response, QByteArray& bytes);
    void handleSignOut(const Response& response, QByteArray& bytes);
    void handleGetUsers(const Response& response, QByteArray& bytes);
    void handleOnline(const Response& response, QByteArray& bytes);
    void handleOffline(const Response& response, QByteArray& bytes);
    void handleChallenge(const Response& response, QByteArray& bytes);
    void handleAccept(const Response& response, QByteArray& bytes);
    void handleDecline(const Response& response, QByteArray& bytes);
    void handleReady(const Response& response, QByteArray& bytes);
    void handleShot(const Response& response, QByteArray& bytes);
    void handleGameOver(const Response& response, QByteArray& bytes);

private:
    Ui::MainWindow* ui;

    QTcpSocket* socket;

    QByteArray buffer;
    User current;

    GameManager* gameManager;
};

#endif // !MAINWINDOW_H
