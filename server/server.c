#include <stdio.h>
#include <libwebsockets.h>
#include <string.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <time.h>
#include <stdlib.h>
#include "constants.h"
#include "crypt.h"
#include "respone.h"
#include "database.h"
#include "server_callback.h"

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

    int rc = sqlite3_open_v2(DATABASE_URL, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, "unix-none");
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    while (1)
    {
        lws_service(context, 50);
    }

    db_close();
    lws_context_destroy(context);

    return 0;
}
