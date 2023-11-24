static struct lws *client = NULL;

void time_string(char *timeString)
{
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(timeString, "%d/%d/%d - %d:%d:%d",
            timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct sockaddr_in addr;
    if (reason == LWS_CALLBACK_ESTABLISHED)

    {
        socklen_t addr_len = sizeof(addr);

        if (getpeername(lws_get_socket_fd(wsi), (struct sockaddr *)&addr, &addr_len) == 0)
        {
            FILE *fp = fopen(FILE_NAME, "a");
            if (fp == NULL)
            {
                fprintf(stderr, "Cannot open file.\n");
                return -1;
            }
            char timeString[80];
            time_string(timeString);
            fprintf(fp, "Client connected from %s [%s]\n", inet_ntoa(addr.sin_addr), timeString);
            fclose(fp);
        }

        client = wsi;
    }
    else if (reason == LWS_CALLBACK_RECEIVE)
    {
        char buffer[BUFF_SIZE];
        memcpy(buffer, in, len);
        buffer[len] = '\0';
        cJSON *jsonRoot = cJSON_Parse(buffer);
        if (jsonRoot == NULL)
        {
            lwsl_err("Failed to parse JSON\n");
            return -1;
        }

        cJSON *request = cJSON_GetObjectItemCaseSensitive(jsonRoot, "request");
        cJSON *data = cJSON_GetObjectItemCaseSensitive(jsonRoot, "data");
        cJSON *respone = cJSON_CreateObject();
        char *req = request->valuestring;
        int result = 0;

        if (strcmp(req, "login") == 0)
            result = find_user(data, respone);
        else if (strcmp(req, "register") == 0)
            result = insert_user(data, respone);

        if (result < 0)
        {
            lwsl_err("Failed to communicate with database\n");
            return -1;
        }

        char *jsonString = cJSON_Print(respone);
        if (client && lws_write(client, jsonString, strlen(jsonString), LWS_WRITE_TEXT) < 0)
        {
            lwsl_err("Failed to send data to a client\n");
            return -1;
        }
    }
    else if (reason == LWS_CALLBACK_CLOSED)
    {
        FILE *fp = fopen(FILE_NAME, "a");
        if (fp == NULL)
        {
            fprintf(stderr, "Cannot open file.\n");
            return -1;
        }
        char timeString[80];
        time_string(timeString);
        fprintf(fp, "Client %s disconnect [%s]\n", inet_ntoa(addr.sin_addr), timeString);
        fclose(fp);
        client = NULL;
    }
    else if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
    {
        char message[] = "Server can read data from user please do something";
        if (client && lws_write(client, message, len, LWS_WRITE_TEXT) < 0)
        {
            lwsl_err("Failed to send data to a client\n");
            return -1;
        }
    }

    return 0;
}