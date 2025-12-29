#include <stdio.h>
#include <string.h>

#include "../headers/util.h"

int parse_url(
    const char *url,
    char *host,
    char *path,
    int *port,
    int *use_ssl
) {
    if (sscanf(url, "https://%255[^/]%1023s", host, path) == 2) {
        *port = 443;
        *use_ssl = 1;
        return 0;
    }
    if (sscanf(url, "http://%255[^/]%1023s", host, path) == 2) {
        *port = 80;
        *use_ssl = 0;
        return 0;
    }
    return -1;
}
