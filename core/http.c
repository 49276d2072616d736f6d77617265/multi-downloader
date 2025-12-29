#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <openssl/ssl.h>

#include "../headers/http.h"
#include "../headers/tcp.h"
#include "../headers/config.h"

static long long get_file_size_internal(
    const char *host, const char *path,
    int port, int use_ssl, int depth
) {
    if (depth > MAX_REDIRECTS) return -1;

    int sock = connect_tcp(host, port);
    if (sock < 0) return -1;

    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;

    if (use_ssl) {
        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        SSL_set_tlsext_host_name(ssl, host);
        SSL_set_fd(ssl, sock);
        if (SSL_connect(ssl) <= 0) goto fail;
    }

    char req[1024];
    snprintf(req, sizeof(req),
        "HEAD %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: multi-downloader\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n\r\n",
        path, host);

    use_ssl ? SSL_write(ssl, req, strlen(req))
            : write(sock, req, strlen(req));

    char header[MAX_HEADER];
    if (read_http_header(sock, ssl, use_ssl, header, sizeof(header)) < 0)
        goto fail;

    dump_http_header("HEAD RESPONSE", header);

    long long size;
    if (sscanf(strcasestr(header, "Content-Length:") ?: "",
               "Content-Length: %lld", &size) == 1) {
        if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
        close(sock);
        return size;
    }

fail:
    if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
    close(sock);
    return -1;
}

long long get_file_size(const char *host, const char *path, int port, int ssl) {
    return get_file_size_internal(host, path, port, ssl, 0);
}

int read_http_header(int sock, SSL *ssl, int use_ssl, char *out, int max) {
    int len = 0, n;
    while (len < max - 1) {
        n = use_ssl ? SSL_read(ssl, out + len, max - len - 1)
                    : read(sock, out + len, max - len - 1);
        if (n <= 0) break;
        len += n;
        out[len] = 0;
        if (strstr(out, "\r\n\r\n"))
            return len;
    }
    return -1;
}

void dump_http_header(const char *title, const char *header) {
    fprintf(stderr,
        "\n%s========== %s ==========%s\n%s%s================================\n",
        C_YELLOW, title, C_RESET,
        header,
        C_YELLOW
    );
}
