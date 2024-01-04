#include "lib.h"

int server_sock;
MYSQL *mysql_conn;

void sig_int(int signo)
{
    close(server_sock);
    mysql_close(mysql_conn);
    exit(EXIT_SUCCESS);
}

int main()
{
    mysql_conn = mysql_init(NULL);

    if (mysql_conn == NULL)
    {
        fprintf(stderr, "mysql_init() failed\n");
        exit(EXIT_FAILURE);
    }

    if (mysql_real_connect(mysql_conn, "127.0.0.1", "root", "038202001252", "ltm", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(mysql_conn);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError creating socket: ");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&(server.sin_zero), 0, 8);

    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError binding socket: ");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_sock, BACK_LOG) == -1)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server running on port %d.\n", PORT);

    // Handle clients
    while (1)
    {
        int client_socket = accept(server_sock, (struct sockaddr *)&client, &sin_size);
        printf("\nYou got connection from %s\n", inet_ntoa(client.sin_addr));

        lock();
        if (getSize(Clients) == BACK_LOG)
            printf("Max clients connected\n");
        else
            addClient(&Clients, createUser(client_socket));
        unlock();

        // Create new thread to handle each client
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *)&client_socket);
    }

    close(server_sock);

    return 0;
}
