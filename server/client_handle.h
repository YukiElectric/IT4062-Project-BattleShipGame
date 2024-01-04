int sendToClient(char *message, int length, int con_sock)
{
    int byte_sents = send(con_sock, message, length, 0);
    if (byte_sents < 0)
    {
        perror("Error sending to client: ");
        return 0;
    }
    return byte_sents;
}

void *client_handler(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUFF_SIZE];
    int byte_receives;
    lock();
    List *client = findClient(Clients, client_socket);
    unlock();

    while (1)
    {
        byte_receives = recv(client_socket, buffer, BUFF_SIZE - 1, 0);
        if (byte_receives <= 0)
            break;
        buffer[byte_receives] = '\0';

        char *jsonStart = strstr(buffer, "\r\n\r\n");
        if (jsonStart != NULL)
        {
            jsonStart += 4; // Move past the empty line
            cJSON *jsonRoot = cJSON_Parse(jsonStart);
            if (jsonRoot == NULL)
            {
                printf("Failed to parse json");
                break;
            }
            cJSON *request = cJSON_GetObjectItemCaseSensitive(jsonRoot, "request");
            cJSON *data = cJSON_GetObjectItemCaseSensitive(jsonRoot, "data");
            cJSON *response = cJSON_CreateObject();
            char *req = request->valuestring;
            int result = 0;

            if (strcmp(req, "login") == 0)
                result = login(data, response);
            else if (strcmp(req, "register") == 0)
                result = insert_user(data, response);
            else if (strcmp(req, "start_match") == 0)
                result = start_match(data, response);
            else if (strcmp(req, "finish_match") == 0)
                result = finish_match(data, response);

            if (result < 0)
            {
                printf("Failed to communicate with database\n");
                break;
            }

            char *jsonString = cJSON_Print(response);
            if (jsonString)
            {
                printf("%s\n", jsonString);
                int jsonLength = strlen(jsonString);

                if (sendToClient(jsonString, jsonLength, client_socket) == 0)
                {
                    printf("Failed to send response to the client\n");
                }
                free(jsonString);
            }
            else
            {
                printf("Failed to create JSON string\n");
            }
        }
        else
        {
            printf("No content found in the received data.\n");
            break;
        }
    }

    lock();
    removeClient(&Clients, client_socket);
    unlock();
    printf("Client %d disconnected\n", client_socket);
    close(client_socket);
    return NULL;
}
