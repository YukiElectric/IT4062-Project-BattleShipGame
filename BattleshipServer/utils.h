#ifndef UTILS_H
#define UTILS_H

#include <cstring>

#define BOARD_SIZE 12

struct User {
    char username[255]{};
    char password[255]{};
    int elo{0};
    // bool quickmatch{false};

    User() = default;

    User(char* username, char* password, int elo) : elo(elo) {
        strncpy(this->username, username, sizeof(this->username));
        strncpy(this->password, password, sizeof(this->username));
    }

    bool operator==(const User& other) const {
        return strcmp(username, other.username) == 0;
    }
};

enum CellType {
    CELL_EMPTY,
    CELL_MISS,
    CELL_HIT,
    CELL_SHIP,
};

struct Move {
    int row{};
    int col{};
    int player{};
    CellType type{};

    Move() = default;
};

struct Match {
    User player1{};
    User player2{};
    int status1{};
    int status2{};
    int turn{};
    int winner{};
    CellType board1[BOARD_SIZE][BOARD_SIZE]{};
    CellType board2[BOARD_SIZE][BOARD_SIZE]{};

    Match() = default;
};

struct Message {
    char message[2048]{};

    Message() = default;
};

enum RequestType {
    REQUEST_NONE,
    REQUEST_SIGN_UP,
    REQUEST_SIGN_IN,
    REQUEST_SIGN_OUT,
    REQUEST_GET_USERS,
    REQUEST_ONLINE,
    REQUEST_OFFLINE,
    REQUEST_CHALLENGE,
    REQUEST_ACCEPT,
    REQUEST_DECLINE,
    REQUEST_READY,
    REQUEST_SHOT,
    REQUEST_GAME_OVER,
    REQUEST_GET_RANK,
    REQUEST_CHAT
};

enum Status {
    STATUS_OK,
    STATUS_ERROR,
    STATUS_UNAUTHORIZED,
};

struct Request {
    RequestType type{};
    User user{};
    User user2{};
    Move move{};
    Message message{};
    int rShips[5]{};
    int cShips[5]{};
    int oriShips[5]{};
};

struct Response {
    RequestType type{};
    Status status{};
    char message[255]{};
    User user{};
    User user2{};
};

#endif // !UTILS_H
