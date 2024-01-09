#include <bits/stdc++.h>
#include <sqlite3.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"

#define MAX_CLIENTS 200
#define IP_LENGTH (INET_ADDRSTRLEN + 7)

using namespace std;

const char* databaseName = "file.db";

int serverFd = -1;

vector<User> userResults;
unordered_map<int, User> clients;
vector<int> clientsFd;
vector<Match> matches;
vector<vector<Move>> moves;

int createDatabase();
int createUserTable();
int userCallback(void* NotUsed, int argc, char** argv, char** azColName);
int insertUser(const char* username, const char* password);
int getUsers();
int getUser(const char* username);
int updateUser(const User& user);

int getMaxFd() {
    int maxFd = serverFd;
    for (int i : clientsFd) {
        if (i > maxFd) {
            maxFd = i;
        }
    }
    return maxFd + 1;
}

int addClient() {
    if (clientsFd.size() == MAX_CLIENTS) {
        return -1;
    }

    struct sockaddr_in clientAddr{};
    socklen_t clientAddrSize = sizeof(clientAddr);

    int clientFd = accept(serverFd, (struct sockaddr*) &clientAddr, &clientAddrSize);
    clientsFd.push_back(clientFd);
    clients.insert({clientFd, User()});

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(clientFd, (struct sockaddr*) &addr, &addr_size);
    char* ipv4 = static_cast<char*>(malloc(IP_LENGTH));
    strcpy(ipv4, inet_ntoa(addr.sin_addr));

    char port[7];
    sprintf(port, ":%hu", ntohs(addr.sin_port));
    strcat(ipv4, port);

    cout << "> addClient(): " << clientFd << ", " << ipv4 << endl;
    return 0;
}

void removeClient(int clientFd) {
    close(clientFd);
    clients.erase(clientFd);
    clientsFd.erase(std::find(clientsFd.begin(), clientsFd.end(), clientFd));
    cout << "> removeClient(): " << clientFd << endl;
}

void removeMatch(const User& user) {
    for (int i = 0; i < matches.size(); i++) {
        if (matches[i].player1 == user || matches[i].player2 == user) {
            matches.erase(matches.begin() + i);
            moves.erase(moves.begin() + i);
            i--;
        }
    }
}

int checkGameOver(const Match& match) {
    int count1 = 0;
    int count2 = 0;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (match.board1[r][c] == CELL_SHIP) {
                count1++;
            }
            if (match.board2[r][c] == CELL_SHIP) {
                count2++;
            }
        }
    }

    if (count1 == 0) {
        return 1;
    }

    if (count2 == 0) {
        return 0;
    }

    return -1;
}

void sendGameOver(User user, Match& match) {
    Response response;
    response.type = REQUEST_GAME_OVER;
    response.status = Status::STATUS_OK;
    strcpy(response.message, "Game over!");

    size_t packageSize = sizeof(size_t) + sizeof(response) + sizeof(match);

    int clientFd = -1;
    for (const auto& client : clients) {
        if (strcmp(client.second.username, user.username) == 0) {
            clientFd = client.first;
            break;
        }
    }
    if (clientFd == -1) {
        return;
    }

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
    send(clientFd, &match, sizeof(match), 0);

    cout << clientFd << "> sendGameOver(): OK" << endl;
}

void sendQuit(User user) {
    int i;
    for (i = 0; i < matches.size(); i++) {
        if (matches[i].player1 == user || matches[i].player2 == user) {
            break;
        }
    }
    if (i == matches.size()) {
        return;
    }

    Match match = matches[i];
    match.winner = user == match.player1 ? 1 : 0;
    sendGameOver(user == match.player1 ? match.player2 : match.player1, match);
}

void notifyOnOff(int clientFd, bool isOn = true) {
    if (strlen(clients.at(clientFd).username) == 0) {
        cout << clientFd << "> notifyOnOff(): Not sign in" << endl;
        return;
    }

    if (!isOn) {
        removeMatch(clients.at(clientFd));
    }

    Response response;
    response.status = Status::STATUS_OK;
    response.type = isOn ? REQUEST_ONLINE : REQUEST_OFFLINE;
    response.user = clients.at(clientFd);
    strcpy(response.message, "Notify on/off!");

    size_t packageSize = sizeof(size_t) + sizeof(response);
    for (const auto& client : clients) {
        if (strlen(client.second.username) > 0 && clientFd != client.first) {
            send(client.first, &packageSize, sizeof(packageSize), 0);
            send(client.first, &response, sizeof(response), 0);
        }
    }
    cout << clientFd << "> notifyOnOff(): Done!" << endl;
}

void handleSignIn(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_ERROR;
    response.user = request.user;

    size_t packageSize = sizeof(size_t) + sizeof(response);
    send(clientFd, &packageSize, sizeof(packageSize), 0);

    if (strlen(request.user.username) == 0 || strlen(request.user.password) == 0) {
        cout << clientFd << "> handleSignIn(): Invalid username or password" << endl;
        strcpy(response.message, "Invalid username or password!");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (getUser(request.user.username) != SQLITE_OK) {
        cout << clientFd << "> handleSignIn(): An error occurred" << endl;
        strcpy(response.message, "An error occurred. Please try again");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (userResults.empty()) {
        cout << clientFd << "> handleSignIn(): User not found" << endl;
        strcpy(response.message, "User not found!");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (strcmp(userResults[0].password, request.user.password) != 0) {
        cout << clientFd << "> handleSignIn(): Invalid password" << endl;
        strcpy(response.message, "Invalid password!");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    for (const auto& client : clients) {
        if (strcmp(client.second.username, request.user.username) == 0) {
            cout << clientFd << "> handleSignIn(): User already sign in" << endl;
            strcpy(response.message, "User already sign in!");
            send(clientFd, &response, sizeof(response), 0);
            return;
        }
    }

    clients.at(clientFd) = userResults[0];

    response.status = Status::STATUS_OK;
    response.user.elo = userResults[0].elo;
    strcpy(response.message, "Sign in successfully!");
    send(clientFd, &response, sizeof(response), 0);
    cout << clientFd << "> handleSignIn(): Sign in successfully" << endl;
}

void handleSignUp(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_ERROR;
    response.user = request.user;

    size_t packageSize = sizeof(size_t) + sizeof(response);
    send(clientFd, &packageSize, sizeof(packageSize), 0);

    if (strlen(request.user.username) == 0 || strlen(request.user.password) == 0) {
        cout << clientFd << "> handleSignUp(): Invalid username or password" << endl;
        strcpy(response.message, "Invalid username or password!");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (getUser(request.user.username) != SQLITE_OK) {
        cout << clientFd << "> handleSignUp(): An error occurred" << endl;
        strcpy(response.message, "An error occurred. Please try again");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (!userResults.empty()) {
        cout << clientFd << "> handleSignUp(): User already exists" << endl;
        strcpy(response.message, "User already exists!");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    if (insertUser(request.user.username, request.user.password) != SQLITE_OK) {
        cout << clientFd << "> handleSignUp(): An error occurred" << endl;
        strcpy(response.message, "An error occurred. Please try again");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    response.status = Status::STATUS_OK;
    strcpy(response.message, "Sign up successfully!");
    send(clientFd, &response, sizeof(response), 0);
    cout << clientFd << "> handleSignUp(): Sign up successfully" << endl;
}

void handleSignOut(int clientFd, Request request) {
    clients.at(clientFd) = User();

    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;

    size_t packageSize = sizeof(size_t) + sizeof(response);
    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
    cout << clientFd << "> handleSignOut(): OK " << packageSize << " " << request.user.username << endl;
}

void handleGetUsers(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    strcpy(response.message, "Get users successfully!");

    vector<User> users;
    for (const auto& client : clients) {
        if (strlen(client.second.username) > 0 && strcmp(client.second.username, request.user.username) != 0) {
            users.push_back(client.second);
        }
    }

    size_t size = users.size();
    size_t packageSize = sizeof(size_t) + sizeof(response) + sizeof(size) + size * sizeof(User);

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
    send(clientFd, &size, sizeof(size), 0);

    for (const auto& user : users) {
        send(clientFd, &user, sizeof(user), 0);
    }

    cout << clientFd << "> handleGetUsers(): Get users successfully (" << size << ")" << endl;
}

void handleGetRank(int clientFd, Request request){
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    strcpy(response.message, "Get rank successfully!");

    userResults.clear();
    if(getUsers()!= SQLITE_OK) {
        cout << clientFd << "> handleGetRank(): An error occurred" << endl;
        strcpy(response.message, "An error occurred. Please try again");
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    size_t size = userResults.size();
    size_t packageSize = sizeof(size_t) + sizeof(response) + sizeof(size) + size * sizeof(User);

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
    send(clientFd, &size, sizeof(size), 0);

    for(const auto& user : userResults) {
        send(clientFd, &user, sizeof(user), 0);
    }

    cout << clientFd << "> handleGetRank(): Get rank successfully (" << size << ")" << endl;
}

void handleChallenge(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Send challenge successfully!");

    size_t packageSize = sizeof(size_t) + sizeof(response);

    int opponentFd = -1;
    for (const auto& client : clients) {
        if (strcmp(client.second.username, request.user2.username) == 0) {
            opponentFd = client.first;
            break;
        }
    }
    if (opponentFd == -1) {
        cout << clientFd << "> handleChallenge(): User not found" << endl;
        response.status = Status::STATUS_ERROR;
        strcpy(response.message, "User not found!");
        send(clientFd, &packageSize, sizeof(packageSize), 0);
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    for (const auto& match : matches) {
        if (match.player1 == request.user2 || match.player2 == request.user2) {
            cout << clientFd << "> handleChallenge(): User is playing" << endl;
            response.status = Status::STATUS_ERROR;
            strcpy(response.message, "User is playing!");
            send(clientFd, &packageSize, sizeof(packageSize), 0);
            send(clientFd, &response, sizeof(response), 0);
            return;
        }
    }

    Match nMatch;
    nMatch.player1 = request.user;
    nMatch.player2 = request.user2;
    nMatch.status1 = 0;
    nMatch.status2 = 0;
    nMatch.turn = -1;
    nMatch.winner = -1;
    matches.push_back(nMatch);
    moves.push_back(vector<Move>());

    send(opponentFd, &packageSize, sizeof(packageSize), 0);
    send(opponentFd, &response, sizeof(response), 0);

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);

    cout << clientFd << "> handleChallenge(): Send challenge successfully" << endl;
}

void handleAccept(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Accepted!");

    size_t packageSize = sizeof(size_t) + sizeof(response);

    int opponentFd = -1;
    for (const auto& client : clients) {
        if (strcmp(client.second.username, request.user2.username) == 0) {
            opponentFd = client.first;
            break;
        }
    }
    if (opponentFd == -1) {
        cout << clientFd << "> handleAccept(): User not found" << endl;
        response.status = Status::STATUS_ERROR;
        strcpy(response.message, "User not found!");
        send(clientFd, &packageSize, sizeof(packageSize), 0);
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    bool found = false;
    for (const auto& match : matches) {
        if (match.player1 == request.user || match.player2 == request.user) {
            found = true;
            break;
        }
    }

    if (!found) {
        cout << clientFd << "> handleAccept(): Match not found" << endl;
        response.status = Status::STATUS_ERROR;
        strcpy(response.message, "Match not found!");
        send(clientFd, &packageSize, sizeof(packageSize), 0);
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    send(opponentFd, &packageSize, sizeof(packageSize), 0);
    send(opponentFd, &response, sizeof(response), 0);

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);

    cout << clientFd << "> handleAccept(): OK" << endl;
}

void handleDecline(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Rejects!");

    size_t packageSize = sizeof(size_t) + sizeof(response);

    int opponentFd = -1;
    for (const auto& client : clients) {
        if (strcmp(client.second.username, request.user2.username) == 0) {
            opponentFd = client.first;
            break;
        }
    }
    if (opponentFd == -1) {
        cout << clientFd << "> handleDecline(): User not found" << endl;
        response.status = Status::STATUS_ERROR;
        strcpy(response.message, "User not found!");
        send(clientFd, &packageSize, sizeof(packageSize), 0);
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    bool found = false;
    for (const auto& match : matches) {
        if (match.player1 == request.user || match.player2 == request.user) {
            found = true;
            break;
        }
    }

    if (!found) {
        cout << clientFd << "> handleDecline(): Match not found" << endl;
        response.status = Status::STATUS_ERROR;
        strcpy(response.message, "Match not found!");
        send(clientFd, &packageSize, sizeof(packageSize), 0);
        send(clientFd, &response, sizeof(response), 0);
        return;
    }

    removeMatch(request.user);

    send(opponentFd, &packageSize, sizeof(packageSize), 0);
    send(opponentFd, &response, sizeof(response), 0);

    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);

    cout << clientFd << "> handleDecline(): OK" << endl;
}

void handleReady(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Ready!");

    size_t packageSize = sizeof(size_t) + sizeof(response);

    for (auto& match : matches) {
        if (match.player1 == request.user || match.player2 == request.user) {
            if (match.player1 == request.user && match.status1 == 1) {
                cout << clientFd << "> handleReady(): You are ready" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "You are ready!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            if (match.player2 == request.user && match.status2 == 1) {
                cout << clientFd << "> handleReady(): You are ready" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "You are ready!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            int opponentFd = -1;
            for (const auto& client : clients) {
                if (match.player1 == request.user && strcmp(client.second.username, match.player2.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
                if (match.player2 == request.user && strcmp(client.second.username, match.player1.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
            }

            if (opponentFd == -1) {
                cout << clientFd << "> handleReady(): User not found" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "User not found!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            for (int r = 0; r < BOARD_SIZE; r++) {
                for (int c = 0; c < BOARD_SIZE; c++) {
                    if (match.player1 == request.user) {
                        match.board1[r][c] = CELL_EMPTY;
                    } else {
                        match.board2[r][c] = CELL_EMPTY;
                    }
                }
            }

            for (int i = 0; i < 5; i++) {
                int size = 0;
                switch (i) {
                    case 0:
                        size = 2;
                        break;

                    case 1:
                    case 2:
                        size = 3;
                        break;

                    case 3:
                        size = 4;
                        break;

                    case 4:
                        size = 5;
                        break;

                    default:
                        size = 0;
                        break;
                }

                int row = request.rShips[i];
                int col = request.cShips[i];
                for (int j = 0; j < size; j++) {
                    if (request.user == match.player1) {
                        if (request.oriShips[i] == 0) {
                            if (match.board1[row + j][col] != CELL_EMPTY) {
                                cout << clientFd << "> handleReady(): Invalid ships" << endl;
                                response.status = Status::STATUS_ERROR;
                                strcpy(response.message, "Invalid ships!");
                                send(clientFd, &packageSize, sizeof(packageSize), 0);
                                send(clientFd, &response, sizeof(response), 0);
                                return;
                            }
                            match.board1[row + j][col] = CELL_SHIP;
                        } else {
                            if (match.board1[row][col + j] != CELL_EMPTY) {
                                cout << clientFd << "> handleReady(): Invalid ships" << endl;
                                response.status = Status::STATUS_ERROR;
                                strcpy(response.message, "Invalid ships!");
                                send(clientFd, &packageSize, sizeof(packageSize), 0);
                                send(clientFd, &response, sizeof(response), 0);
                                return;
                            }
                            match.board1[row][col + j] = CELL_SHIP;
                        }
                    } else {
                        if (request.oriShips[i] == 0) {
                            if (match.board2[row + j][col] != CELL_EMPTY) {
                                cout << clientFd << "> handleReady(): Invalid ships" << endl;
                                response.status = Status::STATUS_ERROR;
                                strcpy(response.message, "Invalid ships!");
                                send(clientFd, &packageSize, sizeof(packageSize), 0);
                                send(clientFd, &response, sizeof(response), 0);
                                return;
                            }
                            match.board2[row + j][col] = CELL_SHIP;
                        } else {
                            if (match.board2[row][col + j] != CELL_EMPTY) {
                                cout << clientFd << "> handleReady(): Invalid ships" << endl;
                                response.status = Status::STATUS_ERROR;
                                strcpy(response.message, "Invalid ships!");
                                send(clientFd, &packageSize, sizeof(packageSize), 0);
                                send(clientFd, &response, sizeof(response), 0);
                                return;
                            }
                            match.board2[row][col + j] = CELL_SHIP;
                        }
                    }
                }
            }

            if (match.player1 == request.user) {
                match.status1 = 1;
            } else {
                match.status2 = 1;
            }

            if (match.status1 == 1 && match.status2 == 1) {
                random_device rd;
                mt19937 rng(rd());
                uniform_int_distribution<int> uni(1, 100);
                auto num = uni(rng);
                match.turn = num % 2;
            }

            packageSize += sizeof(match);

            send(opponentFd, &packageSize, sizeof(packageSize), 0);
            send(opponentFd, &response, sizeof(response), 0);
            send(opponentFd, &match, sizeof(match), 0);

            send(clientFd, &packageSize, sizeof(packageSize), 0);
            send(clientFd, &response, sizeof(response), 0);
            send(clientFd, &match, sizeof(match), 0);

            cout << clientFd << "> handleReady(): OK" << endl;
            for (auto& r : match.board1) {
                for (auto& c : r) {
                    cout << c << " ";
                }
                cout << endl;
            }
            cout << endl;
            for (auto& r : match.board2) {
                for (auto& c : r) {
                    cout << c << " ";
                }
                cout << endl;
            }
            return;
        }
    }

    cout << clientFd << "> handleReady(): Match not found" << endl;
    response.status = Status::STATUS_ERROR;
    strcpy(response.message, "Match not found!");
    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
}

void handleChat(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Sent chat successfully");
    size_t packageSize = sizeof(size_t) + sizeof(response);
    int index = -1;
    for (auto& match : matches) {
        index++;
        if (match.player1 == request.user || match.player2 == request.user) {
            int opponentFd = -1;
            for (const auto& client : clients) {
                if (match.player1 == request.user && strcmp(client.second.username, match.player2.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
                if (match.player2 == request.user && strcmp(client.second.username, match.player1.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
            }

            if (opponentFd == -1) {
                cout << clientFd << "> handleChat(): User not found" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "User not found!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            Message message = request.message;

            packageSize += sizeof(message);

            send(opponentFd, &packageSize, sizeof(packageSize), 0);
            send(opponentFd, &response, sizeof(response), 0);
            send(opponentFd, &message, sizeof(message), 0);

            cout << opponentFd << " " << message.message << endl;

            cout << clientFd << "> handleChat(): OK" << endl;
        }
    }
}

void handleShot(int clientFd, Request request) {
    Response response;
    response.type = request.type;
    response.status = Status::STATUS_OK;
    response.user = request.user;
    response.user2 = request.user2;
    strcpy(response.message, "Shooted!");

    size_t packageSize = sizeof(size_t) + sizeof(response);

    int index = -1;
    for (auto& match : matches) {
        index++;
        if (match.player1 == request.user || match.player2 == request.user) {
            int opponentFd = -1;
            for (const auto& client : clients) {
                if (match.player1 == request.user && strcmp(client.second.username, match.player2.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
                if (match.player2 == request.user && strcmp(client.second.username, match.player1.username) == 0) {
                    opponentFd = client.first;
                    break;
                }
            }

            if (opponentFd == -1) {
                cout << clientFd << "> handleShot(): User not found" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "User not found!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            Move move = request.move;
            if (match.turn == 0 && match.player1 == request.user) {
                if (match.board2[move.row][move.col] == CELL_EMPTY) {
                    match.board2[move.row][move.col] = CELL_MISS;
                    move.type = CELL_MISS;
                    match.turn = 1;
                } else if (match.board2[move.row][move.col] == CELL_SHIP) {
                    match.board2[move.row][move.col] = CELL_HIT;
                    move.type = CELL_HIT;
                    match.turn = 1;
                } else {
                    cout << clientFd << "> handleShot(): Invalid move" << endl;
                    response.status = Status::STATUS_ERROR;
                    strcpy(response.message, "Invalid move!");
                    send(clientFd, &packageSize, sizeof(packageSize), 0);
                    send(clientFd, &response, sizeof(response), 0);
                    return;
                }
            } else if (match.turn == 1 && match.player2 == request.user) {
                if (match.board1[move.row][move.col] == CELL_EMPTY) {
                    match.board1[move.row][move.col] = CELL_MISS;
                    move.type = CELL_MISS;
                    match.turn = 0;
                } else if (match.board1[move.row][move.col] == CELL_SHIP) {
                    match.board1[move.row][move.col] = CELL_HIT;
                    move.type = CELL_HIT;
                    match.turn = 0;
                } else {
                    cout << clientFd << "> handleShot(): Invalid move" << endl;
                    response.status = Status::STATUS_ERROR;
                    strcpy(response.message, "Invalid move!");
                    send(clientFd, &packageSize, sizeof(packageSize), 0);
                    send(clientFd, &response, sizeof(response), 0);
                    return;
                }
            } else {
                cout << clientFd << "> handleShot(): Invalid turn" << endl;
                response.status = Status::STATUS_ERROR;
                strcpy(response.message, "Invalid turn!");
                send(clientFd, &packageSize, sizeof(packageSize), 0);
                send(clientFd, &response, sizeof(response), 0);
                return;
            }

            packageSize += sizeof(move);

            send(opponentFd, &packageSize, sizeof(packageSize), 0);
            send(opponentFd, &response, sizeof(response), 0);
            send(opponentFd, &move, sizeof(move), 0);

            send(clientFd, &packageSize, sizeof(packageSize), 0);
            send(clientFd, &response, sizeof(response), 0);
            send(clientFd, &move, sizeof(move), 0);

            cout << clientFd << "> handleShot(): OK" << endl;

            match.winner = checkGameOver(match);
            if (match.winner != -1) {
                sendGameOver(match.player1, match);
                sendGameOver(match.player2, match);
                matches.erase(matches.begin() + index);
                moves.erase(moves.begin() + index);
            }
            return;
        }
    }

    cout << clientFd << "> handleShot(): Match not found" << endl;
    response.status = Status::STATUS_ERROR;
    strcpy(response.message, "Match not found!");
    send(clientFd, &packageSize, sizeof(packageSize), 0);
    send(clientFd, &response, sizeof(response), 0);
}

int main() {
    if (createDatabase() != SQLITE_OK || createUserTable() != SQLITE_OK) {
        return EXIT_FAILURE;
    }

    cout << "> Init database successfully!" << endl;

    long port = 2209;

    struct sockaddr_in serverAddr{};

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "> Bind error!" << endl;
        exit(0);
    }

    listen(serverFd, SOMAXCONN);

    fd_set readFds;
    Request request;

    cout << "> Starting on port " << port << endl;

    while (true) {
        FD_ZERO(&readFds);
        FD_SET(serverFd, &readFds);
        for (size_t i = 0; i < clientsFd.size(); i++) {
            FD_SET(clientsFd[i], &readFds);
        }

        select(getMaxFd(), &readFds, nullptr, nullptr, nullptr);
        if (FD_ISSET(serverFd, &readFds)) {
            if (addClient() == -1) {
                cerr << "> Server is full!" << endl;
                continue;
            }
        }

        for (size_t i = 0; i < clientsFd.size(); i++) {
            if (FD_ISSET(clientsFd[i], &readFds)) {
                memset(&request, 0x00, sizeof(request));
                ssize_t size = recv(clientsFd[i], &request, sizeof(request), 0);
                if (size > 0) {
                    cout << clientsFd[i] << "> recv(): " << size << ", " << request.type << ", "
                         << request.user.username << ", " << request.user.password << endl;

                    switch (request.type) {
                        case REQUEST_SIGN_IN:
                            handleSignIn(clientsFd[i], request);
                            notifyOnOff(clientsFd[i]);
                            break;

                        case REQUEST_SIGN_UP:
                            handleSignUp(clientsFd[i], request);
                            break;

                        case REQUEST_SIGN_OUT:
                            notifyOnOff(clientsFd[i], false);
                            handleSignOut(clientsFd[i], request);
                            break;

                        case REQUEST_GET_USERS:
                            handleGetUsers(clientsFd[i], request);
                            break;

                        case REQUEST_CHALLENGE:
                            handleChallenge(clientsFd[i], request);
                            break;

                        case REQUEST_ACCEPT:
                            handleAccept(clientsFd[i], request);
                            break;

                        case REQUEST_DECLINE:
                            handleDecline(clientsFd[i], request);
                            break;

                        case REQUEST_READY:
                            handleReady(clientsFd[i], request);
                            break;

                        case REQUEST_SHOT:
                            handleShot(clientsFd[i], request);
                            break;
                        
                        case REQUEST_GET_RANK:
                            handleGetRank(clientsFd[i], request);
                            break;
                        
                        case REQUEST_CHAT:
                            handleChat(clientsFd[i], request);
                            break;

                        default:
                            break;
                    }
                } else {
                    sendQuit(clients.at(clientsFd[i]));
                    notifyOnOff(clientsFd[i], false);
                    removeClient(clientsFd[i]);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int createDatabase() {
    sqlite3* db;

    int code = sqlite3_open(databaseName, &db);
    if (code != SQLITE_OK) {
        cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_close(db);
    return code;
}

int createUserTable() {
    sqlite3* db;
    string sql = "CREATE TABLE IF NOT EXISTS Users(username TEXT NOT NULL PRIMARY KEY, password TEXT NOT NULL, elo int DEFAULT 0);";

    try {
        int code;
        char* errMsg = nullptr;

        code = sqlite3_open(databaseName, &db);
        if (code != SQLITE_OK) {
            cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
            return code;
        }

        code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (code != SQLITE_OK) {
            cerr << "> SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        sqlite3_close(db);
        return code;
    } catch (const exception& e) {
        cerr << "> Exception: " << e.what() << endl;
        return SQLITE_ERROR;
    }
}

int userCallback(void* notUsed, int argc, char** argv, char** azColName) {
    if (argc < 3) {
        cerr << "> Invalid number of columns" << endl;
        return SQLITE_ERROR;
    }

    User user;
    strcpy(user.username, argv[0]);
    strcpy(user.password, argv[1]);
    user.elo = stoi(argv[2]);
    userResults.push_back(user);
    return SQLITE_OK;
}

int insertUser(const char* username, const char* password) {
    sqlite3* db;
    string sql =
            "INSERT INTO Users(username, password) VALUES('" + string(username) + "', '" + string(password) + "');";

    try {
        int code;
        char* errMsg = nullptr;

        code = sqlite3_open(databaseName, &db);
        if (code != SQLITE_OK) {
            cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
            return code;
        }

        code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (code != SQLITE_OK) {
            cerr << "> SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        sqlite3_close(db);
        return code;
    } catch (const exception& e) {
        cerr << "> Exception: " << e.what() << endl;
        return SQLITE_ERROR;
    }
}

int getUsers() {
    sqlite3* db;
    string sql = "SELECT * FROM Users ORDER BY elo DESC;";

    try {
        int code;
        char* errMsg = nullptr;

        code = sqlite3_open(databaseName, &db);
        if (code != SQLITE_OK) {
            cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
            return code;
        }

        userResults.clear();

        code = sqlite3_exec(db, sql.c_str(), userCallback, nullptr, &errMsg);
        if (code != SQLITE_OK) {
            cerr << "> SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        sqlite3_close(db);
        return code;
    } catch (const exception& e) {
        cerr << "> Exception: " << e.what() << endl;
        return SQLITE_ERROR;
    }
}

int getUser(const char* username) {
    sqlite3* db;
    string sql = "SELECT * FROM Users WHERE username = '" + string(username) + "';";

    try {
        int code;
        char* errMsg = nullptr;

        code = sqlite3_open(databaseName, &db);
        if (code != SQLITE_OK) {
            cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
            return code;
        }

        userResults.clear();

        code = sqlite3_exec(db, sql.c_str(), userCallback, nullptr, &errMsg);
        if (code != SQLITE_OK) {
            cerr << "> SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        sqlite3_close(db);
        return code;
    } catch (const exception& e) {
        cerr << "> Exception: " << e.what() << endl;
        return SQLITE_ERROR;
    }
}

int updateUser(const User& user) {
    sqlite3* db;
    string sql = "UPDATE Users SET password = '" + string(user.password) + "', " + "elo = " + to_string(user.elo) +
                 " WHERE username = '" + string(user.username) + "';";

    try {
        int code;
        char* errMsg = nullptr;

        code = sqlite3_open(databaseName, &db);
        if (code != SQLITE_OK) {
            cerr << "> Can't open database: " << sqlite3_errmsg(db) << endl;
            return code;
        }

        code = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (code != SQLITE_OK) {
            cerr << "> SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }

        sqlite3_close(db);
        return code;
    } catch (const exception& e) {
        cerr << "> Exception: " << e.what() << endl;
        return SQLITE_ERROR;
    }
}
