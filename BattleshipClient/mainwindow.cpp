#include "mainwindow.h"

#include "ui_mainwindow.h"



#include <QMessageBox>

#include <QTableWidget>

#include <QPlainTextEdit>

#include <QTimer>

#include <QHostAddress>



#include <cstring>



#include "itemuser.h"

#include "itemhistory.h"

#include "listrank.h"





MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow) {

    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->loginPage);



    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);



    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);

    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);

    connect(socket, &QAbstractSocket::errorOccurred, this, &MainWindow::onErrorOccurred);

    socket->connectToHost(QHostAddress::LocalHost, 7200);

    if (socket->waitForConnected()) {

        qDebug() << "Connected to Server";

    } else {

        QMessageBox::critical(this, "QTcpClient", QString("The following error occurred: %1.").arg(socket->errorString()));

        exit(EXIT_FAILURE);

    }



    connect(ui->btnSignIn, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                QString username = ui->edtUsername->text();

                QString password = ui->edtPassword->text();

                if (username.isEmpty() || password.isEmpty()) {

                    QMessageBox::warning(this, "Battleship", "Please fill all required fields");

                    return;

                }



                Request request;

                request.type = REQUEST_SIGN_IN;

                strcpy(request.user.username, username.toStdString().c_str());

                strcpy(request.user.password, password.toStdString().c_str());

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_SIGN_IN sent";

            } else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnSignUp, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                QString username = ui->edtUsername->text();

                QString password = ui->edtPassword->text();

                if (username.isEmpty() || password.isEmpty()) {

                    QMessageBox::warning(this, "Battleship", "Please fill all required fields");

                    return;

                }



                Request request;

                request.type = REQUEST_SIGN_UP;

                strcpy(request.user.username, username.toStdString().c_str());

                strcpy(request.user.password, password.toStdString().c_str());

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_SIGN_UP sent";

            } else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnSignOut, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                QMessageBox::StandardButton reply;

                reply = QMessageBox::question(this, "Battleship", "Do you want to signout?", QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::No) {

                    return;

                }



                Request request;

                request.type = REQUEST_SIGN_OUT;

                request.user = current;

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_SIGN_OUT sent";

            } else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnGetUsers, &QPushButton::clicked, this, [this]() {

        getUsers();

    });

    connect(ui->btnOk, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                Request request;

                request.type = REQUEST_READY;

                request.user = current;

                for (int i = 0; i < 5; i++) {

                    request.rShips[i] = Ship::y_array[i] / GameManager::RECT_SIZE;

                    request.cShips[i] = Ship::x_array[i] / GameManager::RECT_SIZE;

                    request.oriShips[i] = Ship::orientation_array[i] ? 1 : 0;

                    if (request.oriShips[i] == 1) {

                        request.cShips[i] = request.cShips[i] - Ship::size_array[i];

                    }

                }

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_READY sent";

            } else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnSurrender, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                QMessageBox::StandardButton reply;

                reply = QMessageBox::question(this, "Battleship", "Do you want to surrender?", QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::No) {

                    return;

                }



                Request request;

                request.type = REQUEST_SURRENDER;

                request.user = current;

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_SURRENDER sent";

            } else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnShowRank, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                Request request;

                request.type = REQUEST_GET_RANK;

                request.user = current;

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_GET_RANK sent";

            }else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        }else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->btnQuickMatch, &QPushButton::clicked, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                Request request;

                request.type = REQUEST_QUICK_MATCH;

                request.user = current;

                socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                qDebug() << "REQUEST_QUICK_MATCH sent";

                quickmatch = true;

                QMessageBox msg;

                msg.setText("Watting to find match ...");

                QTimer time;

                int cnt = 0;

                QObject::connect(&time, &QTimer::timeout, [this, &msg, &cnt]() {

                    if(quickmatch) {

                        msg.setText(QString("Wating in %1 seconds ...").arg(cnt++));

                    }else {

                        msg.close();

                    }

                });

                time.start(1000);

                int result = msg.exec();

                if(result == QMessageBox::Ok || result == QMessageBox::Cancel) {

                    request.type = REQUEST_CANCEL;

                    socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                    qDebug() << "REQUEST_CANCEL sent";

                }

            }else {

                 QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        } else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });

    connect(ui->chattxt, &QPlainTextEdit::textChanged, this, [this]() {

        if(socket) {

            if(socket->isOpen()) {

                QString newText = ui->chattxt->toPlainText();

                if (newText.contains(QChar('\n'))) {

                    Request request;

                    request.type = REQUEST_CHAT;

                    request.user = current;

                    strcpy(request.message.message, newText.toStdString().c_str());

                    socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

                    qDebug() << "REQUEST_CHAT sent";

                    ui->chattxt->clear();

                    ui->listchat->addItem("You: "+QString(newText));

                }

            }else {

                QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

                MainWindow::close();

            }

        }else {

            QMessageBox::critical(this, "Battleship", "Not connected");

            MainWindow::close();

        }

    });



    gameManager = new GameManager();

    gameManager->myScene = new QGraphicsScene(this);

    gameManager->enemyScene = new QGraphicsScene(this);



    ui->myView->setSceneRect(0, 0, BOARD_SIZE * GameManager::RECT_SIZE, BOARD_SIZE * GameManager::RECT_SIZE);

    ui->myView->setScene(gameManager->myScene);

    ui->enemyView->setSceneRect(0, 0, BOARD_SIZE * GameManager::RECT_SIZE, BOARD_SIZE * GameManager::RECT_SIZE);

    ui->enemyView->setScene(gameManager->enemyScene);



    gameManager->init();



    connect(gameManager, &GameManager::onShipPlaced, this, &MainWindow::shipPlaced);

    connect(gameManager, &GameManager::onShot, this, &MainWindow::shot);

    connect(gameManager, &GameManager::onSetStatus, this, &MainWindow::setStatusText);



    replayManager = new ReplayManager();

    replayManager->player1Scene = new QGraphicsScene(this);

    replayManager->player2Scene = new QGraphicsScene(this);



    ui->player1View->setSceneRect(0, 0, BOARD_SIZE * ReplayManager::RECT_SIZE, BOARD_SIZE * ReplayManager::RECT_SIZE);

    ui->player1View->setScene(replayManager->player1Scene);

    ui->player2View->setSceneRect(0, 0, BOARD_SIZE * ReplayManager::RECT_SIZE, BOARD_SIZE * ReplayManager::RECT_SIZE);

    ui->player2View->setScene(replayManager->player2Scene);



    connect(replayManager, &ReplayManager::onSetStatus, this, &MainWindow::setReplayStatusText);

    connect(replayManager, &ReplayManager::onShotDone, this, &MainWindow::shotDone);

    connect(ui->btnExit, &QPushButton::clicked, this, [this]() {

        replayManager->count = 0;

        ui->stackedWidget->setCurrentWidget(ui->mainPage);

    });

    connect(ui->btnNext, &QPushButton::clicked, this, [this]() {

        ui->btnNext->setEnabled(false);

        ui->btnPrev->setEnabled(false);

        replayManager->next();

    });

    connect(ui->btnPrev, &QPushButton::clicked, this, [this]() {

        ui->btnNext->setEnabled(false);

        ui->btnPrev->setEnabled(false);

        replayManager->prev();

    });

}



MainWindow::~MainWindow() {

    if (socket->isOpen()) {

        socket->close();

        socket->deleteLater();

    }



    delete ui;

}



void MainWindow::onReadyRead() {

    qDebug() << "onReadyRead()" << socket->bytesAvailable();



    buffer.append(socket->readAll());

    if (buffer.size() <= sizeof(size_t)) {

        return;

    }



    size_t packageSize = 0;

    memcpy(&packageSize, buffer.constData(), sizeof(packageSize));

    qDebug() << "onReadyRead() packageSize:" << packageSize;

    if (packageSize == 0 || buffer.size() < packageSize) {

        return;

    }



    buffer = buffer.mid(sizeof(size_t));

    handleResponse(buffer);

    if (!buffer.isEmpty()) {

        onReadyRead();

    }

}



void MainWindow::onSocketDisconnected() {

    socket->deleteLater();

    socket=nullptr;

    qDebug() << "Disconnected";

}



void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error) {

    switch (error) {

    case QAbstractSocket::RemoteHostClosedError:

        break;



    case QAbstractSocket::HostNotFoundError:

        QMessageBox::information(this, "Battleship", "The host was not found. Please check the host name and port settings.");

        break;



    case QAbstractSocket::ConnectionRefusedError:

        QMessageBox::information(this, "Battleship", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");

        break;



    default:

        QMessageBox::information(this, "Battleship", QString("The following error occurred: %1.").arg(socket->errorString()));

        break;

    }

}



void MainWindow::setStatusText(QString status) {

    ui->txtStatus->setText(status);

}



void MainWindow::setReplayStatusText(QString status) {

    ui->txtReplayStatus->setText(status);

}



void MainWindow::challengeClick(User user) {

    qDebug() << "onChallengeClick():" << user.username << user.elo;

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = REQUEST_CHALLENGE;

            request.user = current;

            request.user2 = user;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << "REQUEST_CHALLENGE sent";

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::itemHistoryClick(Match match) {

    qDebug() << "itemHistoryClick():" << QString::number(match.id) << "|" << QString(match.player1.username) << "|" << QString(match.player2.username);

    QString init1, init2;

    for (int i = 0; i < BOARD_SIZE; i++) {

        init1 += "\t";

        init2 += "\t";

        for (int j = 0; j < BOARD_SIZE; j++) {

            init1 += QString::number(match.init1[i][j]) + " ";

            init2 += QString::number(match.init2[i][j]) + " ";

        }

        init1 += "\n";

        init2 += "\n";

    }

    qDebug().noquote() << init1;

    qDebug().noquote() << init2;



    this->curMatch = match;

    getMoves(match);

}



void MainWindow::notifyChallengeRequest(User user) {

    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "Battleship", QString(user.username) + " sent you a challenge!\nDo you like to accept it?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {

        qDebug() << "notifyChallengeRequest():" << "No";

    } else {

        qDebug() << "notifyChallengeRequest():" << "Yes";

    }

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = (reply == QMessageBox::Yes ? REQUEST_ACCEPT : REQUEST_DECLINE);

            request.user = current;

            request.user2 = user;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << (QString(reply == QMessageBox::Yes ? "REQUEST_ACCEPT" : "REQUEST_DECLINE") + " sent");

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::updateListWidget(User user) {

    auto widget = new ItemUser(this);

    widget->setData(user);



    QObject::connect(widget, &ItemUser::onChallengeClick, this, &MainWindow::challengeClick);



    auto item = new QListWidgetItem();

    item->setSizeHint(QSize(480, 50));



    ui->listWidget->addItem(item);

    ui->listWidget->setItemWidget(item, widget);

    ui->listWidget->update();

}



void MainWindow::updateListWidgetHistory(Match match) {

    auto item = new QListWidgetItem();

    item->setSizeHint(QSize(340, 50));



    auto widget = new ItemHistory(this);

    widget->setData(*item, current, match);



    QObject::connect(widget, &ItemHistory::onClick, this, &MainWindow::itemHistoryClick);



    ui->listWidgetHistory->addItem(item);

    ui->listWidgetHistory->setItemWidget(item, widget);

    ui->listWidgetHistory->update();

    ui->listWidgetHistory->scrollToBottom();

}



void MainWindow::shipPlaced() {

    ui->btnOk->setEnabled(gameManager->allShipsArePlaced());

}



void MainWindow::shot(int row, int col) {

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = REQUEST_SHOT;

            request.user = current;

            request.move.row = row;

            request.move.col = col;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << "REQUEST_SHOT sent";

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::shotDone(int cur, int size) {

    ui->btnPrev->setEnabled(true);

    ui->btnNext->setEnabled(true);

    if (cur == -1) {

        ui->btnPrev->setEnabled(false);

    }

    if (cur == size - 1) {

        ui->btnNext->setEnabled(false);

        if (curMatch.winner == 0) {

            QMessageBox::information(this, "Battleship", QString(curMatch.player1.username) + " win!");

        } else {

            QMessageBox::information(this, "Battleship", QString(curMatch.player2.username) + " win!");

        }

    }

}



void MainWindow::showReplayPage(Match match, QList<Move> moves) {

    ui->btnPrev->setEnabled(false);

    if (moves.empty()) {

        ui->btnNext->setEnabled(false);

    } else {

        ui->btnNext->setEnabled(true);

    }



    setReplayStatusText("Ready!!!");



    ui->txtPlayer1->setText(QString(match.player1.username) + "'s board");

    ui->txtPlayer2->setText(QString(match.player2.username) + "'s board");



    replayManager->init(curMatch, moves);

    ui->stackedWidget->setCurrentWidget(ui->replayPage);

}



void MainWindow::getUsers() {

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = REQUEST_GET_USERS;

            request.user = current;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << "REQUEST_GET_USERS sent";

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::getMatches() {

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = REQUEST_GET_MATCHES;

            request.user = current;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << "REQUEST_GET_MATCHES sent";

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::getMoves(const Match& match) {

    if(socket) {

        if(socket->isOpen()) {

            Request request;

            request.type = REQUEST_GET_MOVES;

            request.user = current;

            request.move.matchID = match.id;

            socket->write(reinterpret_cast<const char*>(&request), sizeof(request));

            qDebug() << "REQUEST_GET_MOVES sent";

        } else {

            QMessageBox::critical(this, "Battleship", "Socket doesn't seem to be opened");

            MainWindow::close();

        }

    } else {

        QMessageBox::critical(this, "Battleship", "Not connected");

        MainWindow::close();

    }

}



void MainWindow::handleResponse(QByteArray& bytes) {

    Response response;

    std::memcpy(&response, bytes.constData(), sizeof(response));

    bytes = bytes.mid(sizeof(response));



    switch (response.type) {

    case REQUEST_SIGN_IN:

        handleSignIn(response, bytes);

        break;



    case REQUEST_SIGN_UP:

        handleSignUp(response, bytes);

        break;



    case REQUEST_SIGN_OUT:

        handleSignOut(response, bytes);

        break;



    case REQUEST_GET_USERS:

        handleGetUsers(response, bytes);

        break;



    case REQUEST_GET_MATCHES:

        handleGetMatches(response, bytes);

        break;



    case REQUEST_GET_MOVES:

        handleGetMoves(response, bytes);

        break;



    case REQUEST_ONLINE:

        handleOnline(response, bytes);

        break;



    case REQUEST_OFFLINE:

        handleOffline(response, bytes);

        break;



    case REQUEST_CHALLENGE:

        handleChallenge(response, bytes);

        break;



    case REQUEST_ACCEPT:

        handleAccept(response, bytes);

        break;



    case REQUEST_DECLINE:

        handleDecline(response, bytes);

        break;



    case REQUEST_READY:

        handleReady(response, bytes);

        break;



    case REQUEST_SHOT:

        handleShot(response, bytes);

        break;



    case REQUEST_GAME_OVER:

        handleGameOver(response, bytes);

        break;



    case REQUEST_QUIT:

        handleQuit(response, bytes);

        break;



    case REQUEST_SURRENDER:

        handleSurrender(response, bytes);

        break;



    case REQUEST_GET_RANK:

        handleGetRank(response, bytes);

        break;



    case REQUEST_CHAT:

        handleChat(response, bytes);

        break;



    case REQUEST_QUICK_MATCH:

        handleQuickMatch(response, bytes);

        break;



    default:

        break;

    }

}



void MainWindow::handleSignIn(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    current = response.user;

    this->setWindowTitle("Battleship - " + QString(current.username));

    ui->edtPassword->setText("");

    ui->stackedWidget->setCurrentWidget(ui->mainPage);

    ui->txtUsername->setText(QString(current.username));



    getUsers();

}



void MainWindow::handleSignUp(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    QMessageBox::information(this, "Battleship", response.message);

}



void MainWindow::handleSignOut(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    current = User();

    this->setWindowTitle("Battleship");

    ui->stackedWidget->setCurrentWidget(ui->loginPage);

}



void MainWindow::handleGetUsers(const Response& response, QByteArray& bytes) {

    if (response.status == STATUS_OK) {

        ui->listWidget->clear();



        size_t size = 0;



        memcpy(&size, bytes.constData(), sizeof(size));

        bytes = bytes.mid(sizeof(size));



        User user;

        for (size_t i = 0; i < size; i++) {

            memcpy(&user, bytes.constData(), sizeof(user));

            bytes = bytes.mid(sizeof(user));



            updateListWidget(user);

        }



        getMatches();

    } else {

        QMessageBox::critical(this, "Battleship", response.message);

        return;

    }

}



void MainWindow::handleGetMatches(const Response& response, QByteArray& bytes) {

    if (response.status == STATUS_OK) {

        ui->listWidgetHistory->clear();



        size_t size = 0;



        memcpy(&size, bytes.constData(), sizeof(size));

        bytes = bytes.mid(sizeof(size));



        Match match;

        for (size_t i = 0; i < size; i++) {

            memcpy(&match, bytes.constData(), sizeof(match));

            bytes = bytes.mid(sizeof(match));



            updateListWidgetHistory(match);

        }

    } else {

        QMessageBox::critical(this, "Battleship", response.message);

        return;

    }

}



void MainWindow::handleGetMoves(const Response& response, QByteArray& bytes) {

    if (response.status == STATUS_OK) {

        QList<Move> moves;



        size_t size = 0;

        memcpy(&size, bytes.constData(), sizeof(size));

        bytes = bytes.mid(sizeof(size));



        for (size_t i = 0; i < size; i++) {

            Move move;

            memcpy(&move, bytes.constData(), sizeof(move));

            bytes = bytes.mid(sizeof(move));



            moves.append(move);

        }



        for (const Move& move : moves) {

            qDebug() << move.player << move.row << move.col << move.type;

        }



        showReplayPage(curMatch, moves);

    } else {

        QMessageBox::critical(this, "Battleship", response.message);

        return;

    }

}



void MainWindow::handleOnline(const Response& response, QByteArray& bytes) {

    if (response.status == STATUS_OK) {

        updateListWidget(response.user);

    }



    qDebug() << "handleOnline():" << (response.user.username + QString(", ") + QString::number(response.user.elo));

}



void MainWindow::handleOffline(const Response& response, QByteArray& bytes) {

    if (response.status == STATUS_OK) {

        for (int i = 0; i < ui->listWidget->count(); i++) {

            QListWidgetItem* item = ui->listWidget->item(i);

            ItemUser* widget = dynamic_cast<ItemUser*>(ui->listWidget->itemWidget(item));

            if (strcmp(widget->getData().username, response.user.username) == 0) {

                delete ui->listWidget->takeItem(i);

                break;

            }

        }

        ui->listWidget->update();

    }



    qDebug() << "handleOffline():" << (response.user.username + QString(", ") + QString::number(response.user.elo));

}



void MainWindow::handleChallenge(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    if (response.user == current) {

        gameManager->init();

        ui->stackedWidget->setCurrentWidget(ui->matchPage);

        ui->btnOk->setVisible(true);

        ui->btnSurrender->setVisible(false);

        ui->matchPage->setEnabled(false);

        ui->txtMe->setText(response.user.username);

        ui->txtEnemy->setText(response.user2.username);

        setStatusText("Waiting for opponent's response request");

    } else {

        notifyChallengeRequest(response.user);

    }

}



void MainWindow::handleAccept(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    if (response.user == current) {

        ui->txtMe->setText(response.user.username);

        ui->txtEnemy->setText(response.user2.username);

    }else {

        ui->txtMe->setText(response.user2.username);

        ui->txtEnemy->setText(response.user.username);

    }



    gameManager->init();

    ui->stackedWidget->setCurrentWidget(ui->matchPage);

    ui->matchPage->setEnabled(true);

    ui->btnOk->setVisible(true);

    ui->btnSurrender->setVisible(false);

    setStatusText("Placing ships");

}



void MainWindow::handleDecline(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    if (!(response.user == current)) {

        ui->stackedWidget->setCurrentWidget(ui->mainPage);

        QMessageBox::information(this, "Battleship", QString(response.user.username) + " rejects your challenge!");

    }

}



void MainWindow::handleReady(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    Match match;

    memcpy(&match, bytes.constData(), sizeof(match));

    bytes = bytes.mid(sizeof(match));



    if (match.status1 == 1 && match.status2 == 1) {

        setStatusText("Running...");

        if ((current == match.player1 && match.turn == 0) || (current == match.player2 && match.turn == 1)) {

            gameManager->myTurn = true;

        } else {

            gameManager->myTurn = false;

        }

        gameManager->clearShips();

        for (int r = 0; r < BOARD_SIZE; r++) {

            for (int c = 0; c < BOARD_SIZE; c++) {

                if (current == match.player1 && match.board1[r][c] == CELL_SHIP) {

                    gameManager->myBoard[r][c]->type = CELL_SHIP;

                }

                if (current == match.player2 && match.board2[r][c] == CELL_SHIP) {

                    gameManager->myBoard[r][c]->type = CELL_SHIP;

                }

            }

        }

        if (gameManager->myTurn) {

            setStatusText("Your turn!");

        } else {

            setStatusText("Opponent's turn!");

        }



        gameManager->isRunning = true;

        gameManager->myScene->update();

        ui->matchPage->setEnabled(true);

        ui->btnOk->setVisible(false);

        ui->btnSurrender->setVisible(true);

    } else if (response.user == current) {

        setStatusText("Waiting opponent...");

        ui->matchPage->setEnabled(false);

    }

}



void MainWindow::handleShot(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    Move move;

    memcpy(&move, bytes.constData(), sizeof(move));

    bytes = bytes.mid(sizeof(move));



    qDebug() << "handleShot():" << move.type;



    if (response.user == current) {

        gameManager->myShotDone(move.type);

    } else {

        gameManager->enemyShot(move.row, move.col, move.type);

    }

}



void MainWindow::handleGameOver(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    Match match;

    memcpy(&match, bytes.constData(), sizeof(match));

    bytes = bytes.mid(sizeof(match));



    if ((current == match.player1 && match.winner == 0) || (current == match.player2 && match.winner == 1)) {

        QMessageBox::information(this, "Battleship", "You win!");

    } else {

        QMessageBox::information(this, "Battleship", "You lose!");

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Battleship", "Do you want to play again with"+QString(match.player2.username), QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::Yes) {

            challengeClick(match.player2);

            ui->stackedWidget->setCurrentWidget(ui->matchPage);

            gameManager->init();

        }

    }



    gameManager->isRunning = false;

    gameManager->clear();



    ui->stackedWidget->setCurrentWidget(ui->mainPage);



    updateListWidgetHistory(match);

}



void MainWindow::handleQuit(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    Match match;

    memcpy(&match, bytes.constData(), sizeof(match));

    bytes = bytes.mid(sizeof(match));



    if (gameManager->isRunning) {

        QMessageBox::information(this, "Battleship", "Opponents quit. You win!");

        updateListWidgetHistory(match);

    } else {

        QMessageBox::information(this, "Battleship", "Opponents quit!");

    }



    gameManager->isRunning = false;

    gameManager->clear();



    ui->stackedWidget->setCurrentWidget(ui->mainPage);



    for (int i = 0; i < ui->listWidget->count(); i++) {

        QListWidgetItem* item = ui->listWidget->item(i);

        ItemUser* widget = dynamic_cast<ItemUser*>(ui->listWidget->itemWidget(item));

        if (strcmp(widget->getData().username, response.user2.username) == 0) {

            delete ui->listWidget->takeItem(i);

            break;

        }

    }

    ui->listWidget->update();

}



void MainWindow::handleSurrender(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }



    Match match;

    memcpy(&match, bytes.constData(), sizeof(match));

    bytes = bytes.mid(sizeof(match));



    if (gameManager->isRunning && response.user2 == current) {

        QMessageBox::information(this, "Battleship", "Opponents surrender. You win!");

    }



    gameManager->isRunning = false;

    gameManager->clear();



    ui->stackedWidget->setCurrentWidget(ui->mainPage);



    updateListWidgetHistory(match);

}



void MainWindow::handleGetRank(const Response& response, QByteArray& bytes) {

    listrank *w = new listrank();

    if (response.status == STATUS_OK) {

        size_t size = 0;



        memcpy(&size, bytes.constData(), sizeof(size));

        bytes = bytes.mid(sizeof(size));



        QTableWidget *table = w->findChild<QTableWidget*>("rank");

        User user;

        for (size_t i = 0; i < size; i++) {

            memcpy(&user, bytes.constData(), sizeof(user));

            bytes = bytes.mid(sizeof(user));

            int rowCount = table->rowCount();

            table->insertRow(rowCount);

            table->setItem(i,0, new QTableWidgetItem(QString(user.username)));

            table->setItem(i,1, new QTableWidgetItem(QString::number(user.elo)));

        }

        w->setWindowTitle("Rank");

        w->show();

    }

}



void MainWindow::handleChat(const Response& response, QByteArray& bytes) {

    if (response.status != STATUS_OK) {

        QMessageBox::warning(this, "Battleship", response.message);

        return;

    }

    Message message;

    memcpy(&message, bytes.constData(), sizeof(message));

    bytes = bytes.mid(sizeof(message));

    ui->listchat->addItem(QString(message.message));

}



void MainWindow::handleQuickMatch(const Response& response, QByteArray& bytes) {

    if(response.status == STATUS_OK) {

        quickmatch = false;

        ui->stackedWidget->setCurrentWidget(ui->matchPage);

        gameManager->init();

        ui->matchPage->setEnabled(true);

        ui->btnSurrender->setVisible(false);

        ui->btnOk->setVisible(true);

        setStatusText("Placing ships");

    }

}