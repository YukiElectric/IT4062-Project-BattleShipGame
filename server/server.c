#include "lib.h"

int server_sock;

void sig_int(int signo) {
    close(server_sock);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, sig_int);

    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size = sizeof(struct sockaddr_in);

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {  /* calls socket() */
        perror("\nError: ");
        exit(0);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);

    // Bind socket
    if (bind(server_sock, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("\nError: ");
        exit(0);
    }

    // Listen
    if (listen(server_sock, BACK_LOG) == -1) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server running on port %d.\n", PORT);


    // Handle clients
    while (1) {
        int client_socket = accept(server_sock, (struct sockaddr *) &client, &sin_size);
        printf("\nYou got connection from %s\n", inet_ntoa(client.sin_addr));

        // Add client to list
        lock();
        if(getSize(Clients)==BACK_LOG) printf("Max client connected\n");
        else addClient(&Clients, createUser(client_socket));
        unlock();

        // Create new thread to handle each client
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *) &client_socket);
    }

    close(server_sock);


    return 0;
}
