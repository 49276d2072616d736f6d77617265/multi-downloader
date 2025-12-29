#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdatomic.h>

#include <openssl/ssl.h>

#include "../headers/downloader.h"
#include "../headers/tcp.h"
#include "../headers/http.h"
#include "../headers/config.h"

atomic_ullong downloaded = 0;

void *download_range(void *arg) {
    thread_data_t *d = arg;
    int sock = connect_tcp(d->host, d->port);
    if (sock < 0) return NULL;

    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;

    if (d->use_ssl) {
        ctx = SSL_CTX_new(TLS_client_method());
        ssl = SSL_new(ctx);
        SSL_set_tlsext_host_name(ssl, d->host);
        SSL_set_fd(ssl, sock);
        if (SSL_connect(ssl) <= 0) return NULL;
    }

    char req[1024];
    snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Range: bytes=%lld-%lld\r\n"
        "Connection: close\r\n\r\n",
        d->path, d->host, d->start, d->end);

    d->use_ssl ? SSL_write(ssl, req, strlen(req))
               : write(sock, req, strlen(req));

    char buf[BUF_SIZE];
    char header[MAX_HEADER];
    int hlen = 0, header_done = 0;
    long long offset = d->start;
    int n;

    while ((n = d->use_ssl ? SSL_read(ssl, buf, sizeof(buf))
                           : read(sock, buf, sizeof(buf))) > 0) {

        if (!header_done) {
            memcpy(header + hlen, buf, n);
            hlen += n;
            header[hlen] = 0;

            char *body = strstr(header, "\r\n\r\n");
            if (!body) continue;

            body += 4;
            int blen = hlen - (body - header);

            if (blen > 0) {
                pwrite(d->fd, body, blen, offset);
                offset += blen;
                atomic_fetch_add(&downloaded, blen);
            }
            header_done = 1;
            continue;
        }

        pwrite(d->fd, buf, n, offset);
        offset += n;
        atomic_fetch_add(&downloaded, n);
    }

    if (d->use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
    close(sock);
    return NULL;
}
