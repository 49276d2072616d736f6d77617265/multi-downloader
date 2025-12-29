#pragma once

#include <openssl/ssl.h>

long long get_file_size(const char *host, const char *path, int port, int ssl);
int read_http_header(int sock, SSL *ssl, int use_ssl, char *out, int max);
void dump_http_header(const char *title, const char *header);
