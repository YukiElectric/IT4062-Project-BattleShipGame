void successfully(char *message, cJSON *res){
    cJSON_AddStringToObject(res, "status", "success");
    cJSON_AddStringToObject(res, "message", message);
}