#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "../headers/tcp.h"

int connect_tcp(const char *host, int port) {
    struct addrinfo hints = {0}, *res, *p;
    char portstr[16];
    int sock = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(host, portstr, &hints, &res) != 0)
        return -1;

    for (p = res; p; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) continue;

        struct timeval tv = {20, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(sock);
        sock = -1;
    }

    freeaddrinfo(res);
    return sock;
}
