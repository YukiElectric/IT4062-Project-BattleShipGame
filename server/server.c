#include <libwebsockets.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 5500
#define MAX_CLIENTS 100
#define BUFF_SIZE 4096

static struct lws *clients[MAX_CLIENTS] = {NULL};

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    if (reason == LWS_CALLBACK_ESTABLISHED)
    {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);

        if (getpeername(lws_get_socket_fd(wsi), (struct sockaddr *)&addr, &addr_len) == 0)
        {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
            printf("Client connected from %s\n", ip);
        }

        // Add the new client to the clients array
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i])
            {
                clients[i] = wsi;
                break;
            }
        }
    }
    else if (reason == LWS_CALLBACK_RECEIVE)
    {
        char buffer[BUFF_SIZE];
        memcpy(buffer, in, len);
        buffer[len] = '\0';
        printf("Received from client: %s\n", buffer);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] && lws_write(clients[i], buffer, len, LWS_WRITE_TEXT) < 0)
            {
                lwsl_err("Failed to send data to a client\n");
                return -1;
            }
        }
    }
    else if (reason == LWS_CALLBACK_CLOSED)
    {
        printf("Client disconnected\n");

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == wsi)
            {
                clients[i] = NULL;
                break;
            }
        }
    }
    else if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
    {
        char message[] = "Server can read data from user please do something";
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] && lws_write(clients[i], message, len, LWS_WRITE_TEXT) < 0)
            {
                lwsl_err("Failed to send data to a client\n");
                return -1;
            }
        }
    }

    return 0;
}

int main()
{
    struct lws_context *context;
    struct lws_context_creation_info info;
    const char *iface = NULL;
    struct lws_protocols protocols[] = {
        {"echo-protocol", callback, 0, 0},
        {NULL, NULL, 0, 0}};

    memset(&info, 0, sizeof(info));
    info.port = PORT;
    info.iface = iface;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (!context)
    {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }

    printf("Server running on port %d\n", PORT);

    while (1)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);

    return 0;
}
