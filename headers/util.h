#pragma once

int parse_url(
    const char *url,
    char *host,
    char *path,
    int *port,
    int *use_ssl
);
