#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <qhostaddress.h>
#include <cstring>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(socket, &QAbstractSocket::errorOccurred, this, &MainWindow::onErrorOccurred);
    socket->connectToHost(QHostAddress::LocalHost, 2209);
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
    }

    qDebug() << "handleOffline():" << (response.user.username + QString(", ") + QString::number(response.user.elo));
}

void MainWindow::handleChallenge(const Response& response, QByteArray& bytes) {
    if (response.status != STATUS_OK) {
        QMessageBox::warning(this, "Battleship", response.message);
        return;
    }

    if (response.user == current) {
        ui->stackedWidget->setCurrentWidget(ui->matchPage);
        ui->matchPage->setEnabled(false);
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
        ui->stackedWidget->setCurrentWidget(ui->matchPage);
        gameManager->init();
    }

    ui->matchPage->setEnabled(true);
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

    gameManager->isRunning = false;

    if ((current == match.player1 && match.winner == 0) || (current == match.player2 && match.winner == 1)) {
        QMessageBox::information(this, "Battleship", "You win!");
    } else {
        QMessageBox::information(this, "Battleship", "You lose!");
    }
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}
