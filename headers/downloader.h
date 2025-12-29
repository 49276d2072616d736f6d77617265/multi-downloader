#pragma once

#include <stdatomic.h>

typedef struct {
    char host[256];
    char path[1024];
    int port;
    int use_ssl;
    long long start;
    long long end;
    int fd;
} thread_data_t;

extern atomic_ullong downloaded;

void *download_range(void *arg);
