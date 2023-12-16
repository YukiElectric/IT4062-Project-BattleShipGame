#define PORT 5500
#define BUFF_SIZE 4096
#define BACK_LOG 1000
const char *SECRET_KEY = "IT4062";
const char *DATABASE_URL = "../database/battleship.db";
const char *INSERT_USER = "INSERT INTO users (user_name, password, email) VALUES (?, ?, ?);";
const char *FIND_USER = "SELECT password FROM users WHERE user_name = ? OR email = ?;";
const char *FILE_NAME = "log.txt";