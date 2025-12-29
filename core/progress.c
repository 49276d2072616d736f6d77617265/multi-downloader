#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdatomic.h>

#include "../headers/progress.h"
#include "../headers/downloader.h"
#include "../headers/config.h"

/* DEFINIÇÕES globais */
struct timeval start_time;
long long total_size = 0;
int running = 1;

static double elapsed_seconds() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start_time.tv_sec) +
           (now.tv_usec - start_time.tv_usec) / 1000000.0;
}

static void format_time(double seconds, char *out, size_t len) {
    int h = seconds / 3600;
    int m = ((int)seconds % 3600) / 60;
    int s = (int)seconds % 60;
    snprintf(out, len, "%02d:%02d:%02d", h, m, s);
}

void *progress_thread(void *arg) {
    (void)arg;

    while (running) {
        unsigned long long d = atomic_load(&downloaded);
        if (d > (unsigned long long)total_size)
            d = total_size;

        double elapsed = elapsed_seconds();
        double speed = elapsed > 0 ? d / elapsed : 0;
        double remaining = speed > 0 ? (total_size - d) / speed : 0;

        char elapsed_str[32], eta_str[32];
        format_time(elapsed, elapsed_str, sizeof(elapsed_str));
        format_time(remaining, eta_str, sizeof(eta_str));

        double pct = (double)d * 100.0 / total_size;
        if (pct > 100.0) pct = 100.0;

        printf(
            "\r%sProgress:%s %.2f%% | %s%.2f MB/s%s | Elapsed %s | ETA %s",
            C_BLUE, C_RESET,
            pct,
            C_GREEN, speed / (1024*1024), C_RESET,
            elapsed_str,
            eta_str
        );

        fflush(stdout);
        sleep(1);
    }
    return NULL;
}
