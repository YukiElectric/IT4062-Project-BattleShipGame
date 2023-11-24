sqlite3 *db = NULL;
sqlite3_stmt *stmt = NULL;

void db_close()
{
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int insert_user(cJSON *req, cJSON *res)
{
    int rc = sqlite3_prepare_v2(db, INSERT_USER, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error query: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    char *encrypt_password = encrypt(cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring);
    sqlite3_bind_text(stmt, 1, cJSON_GetObjectItemCaseSensitive(req, "user_name")->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, encrypt_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, cJSON_GetObjectItemCaseSensitive(req, "email")->valuestring, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
    {
        char *message = "create account succesfully";
        successfully(message, res);
    }
    else
    {
        char *message = "create account fail";
        successfully(message, res);
    }
    return 1;
}

int find_user(cJSON *req, cJSON *res)
{
    int rc = sqlite3_prepare_v2(db, FIND_USER, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error query: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_text(stmt, 1, cJSON_GetObjectItemCaseSensitive(req, "user_name")->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cJSON_GetObjectItemCaseSensitive(req, "user_name")->valuestring, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW)
    {
        char *message = "login fail";
        successfully(message, res);
    }
    else
    {
        const char *password = (const char *)sqlite3_column_text(stmt, 0);
        char *decrypt_password = decrypt(password);
        if (strcmp(cJSON_GetObjectItemCaseSensitive(req, "password")->valuestring, decrypt_password) == 0)
        {
            char *message = "login successfully";
            successfully(message, res);
        }
        else
        {
            char *message = "login fail";
            successfully(message, res);
        }
    }
    return 1;
}
