void successfully(char *message, cJSON *res)
{
    cJSON_AddStringToObject(res, "status", "success");
    cJSON_AddStringToObject(res, "message", message);
}

void fail(char *message, cJSON *res)
{
    cJSON_AddStringToObject(res, "status", "fail");
    cJSON_AddStringToObject(res, "message", message);
}

void successfully_with_battle_id(int battle_id, cJSON *res)
{
    cJSON_AddStringToObject(res, "status", "success");
    cJSON_AddNumberToObject(res, "battle_id", battle_id);
}

void successfully_with_user_id(int user_id, cJSON *res)
{
    cJSON_AddStringToObject(res, "status", "success");
    cJSON_AddNumberToObject(res, "user_id", user_id);
}