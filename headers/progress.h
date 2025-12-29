#pragma once

#include <sys/time.h>

extern struct timeval start_time;
extern long long total_size;
extern int running;

void *progress_thread(void *arg);
