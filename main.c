#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <openssl/ssl.h>

#include "headers/util.h"
#include "headers/http.h"
#include "headers/downloader.h"
#include "headers/progress.h"
#include "headers/config.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <url> <output> <threads>\n", argv[0]);
        return 1;
    }

    char host[256], path[1024];
    int port, ssl;

    if (parse_url(argv[1], host, path, &port, &ssl) != 0) {
        fprintf(stderr, "Invalid URL\n");
        return 1;
    }

    SSL_library_init();
    SSL_load_error_strings();

    total_size = get_file_size(host, path, port, ssl);
    if (total_size <= 0) {
        fprintf(stderr, "Failed to get file size\n");
        return 1;
    }

    printf("File size: %lld bytes\n", total_size);

    int fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ftruncate(fd, total_size);

    int threads = atoi(argv[3]);
    pthread_t tid[threads], prog;
    thread_data_t data[threads];

    gettimeofday(&start_time, NULL);
    pthread_create(&prog, NULL, progress_thread, NULL);

    long long chunk = total_size / threads;

    for (int i = 0; i < threads; i++) {
        data[i] = (thread_data_t){
            .port = port,
            .use_ssl = ssl,
            .start = i * chunk,
            .end = (i == threads - 1) ? total_size - 1 : (i + 1) * chunk - 1,
            .fd = fd
        };
        strcpy(data[i].host, host);
        strcpy(data[i].path, path);
        pthread_create(&tid[i], NULL, download_range, &data[i]);
    }

    for (int i = 0; i < threads; i++)
        pthread_join(tid[i], NULL);

    running = 0;
    pthread_join(prog, NULL);

    printf("\n%sDownload complete.%s\n", C_GREEN, C_RESET);
    close(fd);
    return 0;
}
