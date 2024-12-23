#include <sys/socket.h>
#include <netinet/in.h>

#include "context.h"

struct webserver {
    struct server_context* ctx;

    int fd;
    struct sockaddr_in address;
};

struct webserver* create_server(int port, char* web_root_path);
int run_server(struct webserver* srv);
