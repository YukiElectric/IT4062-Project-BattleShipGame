int sendToClient(char *message, int con_sock) {
    int byte_sents;
    byte_sents = send(con_sock, message, strlen(message), 0);
    if(byte_sents<0) return 0;
    return 1;
}

void *client_handler(void *arg) {
    int client_socket = *(int *) arg;
    char buffer[BUFF_SIZE];
    int byte_receives;
    lock();
    List *client = findClient(Clients, client_socket);
    unlock();

    while (1) {
        byte_receives = recv(client_socket, buffer , BUFF_SIZE-1, 0);
        if (byte_receives <= 0) break;
        buffer[byte_receives] = '\0';

        cJSON *jsonRoot = cJSON_Parse(buffer);
        if (jsonRoot == NULL) {
            printf("Falied to parse json");
            break;
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
            printf("Failed to communicate with database\n");
            break;
        }

        char *jsonString = cJSON_Print(respone);
        if(sendToClient(jsonString, client_socket)== 0) break;
    }

    lock();
    removeClient(&Clients,client_socket);
    unlock();
    printf("Client %d disconnected\n", client_socket);
    close(client_socket);
    return NULL;
}