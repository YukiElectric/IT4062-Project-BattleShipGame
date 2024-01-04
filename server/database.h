#include <mysql/mysql.h>

extern MYSQL *mysql_conn;

MYSQL_STMT *stmt;

void db_close()
{
    mysql_stmt_close(stmt);
}

int insert_user(cJSON *req, cJSON *res)
{
    stmt = mysql_stmt_init(mysql_conn);
    if (!stmt)
    {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, "INSERT INTO users (user_name, email, password) VALUES (?, ?, ?)", strlen("INSERT INTO users (user_name, email, password) VALUES (?, ?, ?)")) != 0)
    {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        return -1;
    }

    // Bind parameters
    MYSQL_BIND mysql_bind[3];
    memset(mysql_bind, 0, sizeof(mysql_bind));

    mysql_bind[0].buffer_type = MYSQL_TYPE_STRING;
    mysql_bind[0].buffer = cJSON_GetObjectItemCaseSensitive(req, "user_name")->valuestring;
    mysql_bind[0].buffer_length = strlen(cJSON_GetObjectItemCaseSensitive(req, "user_name")->valuestring);

    mysql_bind[1].buffer_type = MYSQL_TYPE_STRING;
    mysql_bind[1].buffer = cJSON_GetObjectItemCaseSensitive(req, "email")->valuestring;
    mysql_bind[1].buffer_length = strlen(cJSON_GetObjectItemCaseSensitive(req, "email")->valuestring);

    // Binding for password
    mysql_bind[2].buffer_type = MYSQL_TYPE_STRING;
    mysql_bind[2].buffer = cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring;
    mysql_bind[2].buffer_length = strlen(cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring);

    if (mysql_stmt_bind_param(stmt, mysql_bind) != 0)
    {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        return -1;
    }

    mysql_stmt_close(stmt);

    char *message = "create account successfully";
    successfully(message, res);

    return 1;
}

int login(cJSON *req, cJSON *res)
{
    stmt = mysql_stmt_init(mysql_conn);
    if (!stmt)
    {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, "SELECT user_id FROM users WHERE email = ? AND password = ?", strlen("SELECT user_id FROM users WHERE email = ? AND password = ?")) != 0)
    {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        return -1;
    }

    // Bind parameters
    MYSQL_BIND mysql_bind[2];
    memset(mysql_bind, 0, sizeof(mysql_bind));

    mysql_bind[0].buffer_type = MYSQL_TYPE_STRING;
    mysql_bind[0].buffer = cJSON_GetObjectItemCaseSensitive(req, "email")->valuestring;
    mysql_bind[0].buffer_length = strlen(cJSON_GetObjectItemCaseSensitive(req, "email")->valuestring);

    mysql_bind[1].buffer_type = MYSQL_TYPE_STRING;
    mysql_bind[1].buffer = cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring;
    mysql_bind[1].buffer_length = strlen(cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring);

    if (mysql_stmt_bind_param(stmt, mysql_bind) != 0)
    {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "mysql_stmt_execute() failed\n");
        return -1;
    }

    MYSQL_BIND result_bind;
    memset(&result_bind, 0, sizeof(result_bind));

    result_bind.buffer_type = MYSQL_TYPE_LONG;
    result_bind.buffer = malloc(sizeof(int));
    result_bind.buffer_length = sizeof(int);

    if (mysql_stmt_bind_result(stmt, &result_bind) != 0)
    {
        fprintf(stderr, "mysql_stmt_bind_result() failed\n");
        free(result_bind.buffer);
        return -1;
    }

    if (mysql_stmt_fetch(stmt) != 0)
    {
        char *message = "login fail";
        fail(message, res);
    }
    else
    {
        long user_id;
        memcpy(&user_id, result_bind.buffer, sizeof(int));

        if (!user_id)
        {
            char *message = "login fail";
            fail(message, res);
        }
        else
        {
            successfully_with_user_id(user_id, res);
        }
    }

    free(result_bind.buffer);
    mysql_stmt_close(stmt);

    return 1;
}

int start_match(cJSON *req, cJSON *res)
{
    int battle_id;

    stmt = mysql_stmt_init(mysql_conn);
    if (!stmt)
    {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    const char *insert_query = "INSERT INTO battles (player_one, player_two) VALUES (?, ?)";
    if (mysql_stmt_prepare(stmt, insert_query, strlen(insert_query)) != 0)
    {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        return -1;
    }

    MYSQL_BIND mysql_bind[2];
    memset(mysql_bind, 0, sizeof(mysql_bind));

    mysql_bind[0].buffer_type = MYSQL_TYPE_LONG;
    mysql_bind[0].buffer = &cJSON_GetObjectItemCaseSensitive(req, "player_one")->valueint;

    mysql_bind[1].buffer_type = MYSQL_TYPE_LONG;
    mysql_bind[1].buffer = &cJSON_GetObjectItemCaseSensitive(req, "player_two")->valueint;

    if (mysql_stmt_bind_param(stmt, mysql_bind) != 0)
    {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        return -1;
    }

    battle_id = mysql_stmt_insert_id(stmt);
    successfully_with_battle_id(battle_id, res);

    mysql_stmt_close(stmt);

    return 1;
}

int finish_match(cJSON *req, cJSON *res)
{
    stmt = mysql_stmt_init(mysql_conn);
    if (!stmt)
    {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        return -1;
    }

    const char *update_query = "UPDATE battles SET result = ?, ended_at = CURRENT_TIMESTAMP WHERE battle_id = ?";
    if (mysql_stmt_prepare(stmt, update_query, strlen(update_query)) != 0)
    {
        fprintf(stderr, "mysql_stmt_prepare() failed\n");
        return -1;
    }

    MYSQL_BIND mysql_bind[2];
    memset(mysql_bind, 0, sizeof(mysql_bind));

    mysql_bind[0].buffer_type = MYSQL_TYPE_LONG;
    mysql_bind[0].buffer = &cJSON_GetObjectItemCaseSensitive(req, "result")->valueint;

    mysql_bind[1].buffer_type = MYSQL_TYPE_LONG;
    mysql_bind[1].buffer = &cJSON_GetObjectItemCaseSensitive(req, "battle_id")->valueint;

    if (mysql_stmt_bind_param(stmt, mysql_bind) != 0)
    {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        return -1;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        return -1;
    }

    char *message = "Match finished successfully";
    successfully(message, res);

    mysql_stmt_close(stmt);

    return 1;
}
