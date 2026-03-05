/*
 * test_thread_safety.c
 *
 * Cross-platform thread safety test for Log / print
 * - POSIX: pthreads
 * - Windows: CreateThread
 *
 * Expected behavior:
 *   - No crashes
 *   - No deadlocks
 *   - No interleaved lines
 *   - Log file remains valid
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "logging_util.h"

#define THREAD_COUNT 4
#define ITERATIONS 20

Log *logger;

#if PLATFORM_WINDOWS
    #include <windows.h>
    DWORD WINAPI thread_print_loop(LPVOID arg) {
        int thread_id = (int)(intptr_t)arg;
        char *msg;
        for (int j = 0; j < ITERATIONS; j++) {
            print(logger, fmt("thread %d iteration %d", thread_id, j), .i=1);
        }
        return 0;
    }
    int test_thread_safety() {
        HANDLE threads[THREAD_COUNT];
        for (int j = 0; j < THREAD_COUNT; j++) {
            threads[j] = CreateThread(
                NULL,
                0,
                thread_print_loop,
                (LPVOID)(intptr_t)j,
                0,
                NULL
            );
            if (!threads[j]) {
                fprintf(stderr, "CreateThread failed\n");
                return 1;
            }
        }
        WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);
        for (int j = 0; j < THREAD_COUNT; j++)
            CloseHandle(threads[j]);
        return 0;
    }
#else
    #include <pthread.h>
    void *thread_print_loop(void *arg) {
        int thread_id = (int)(intptr_t)arg;
        char *msg;
        for (int j = 0; j < ITERATIONS; j++) {
            print(logger, fmt("thread %d iteration %d", thread_id, j), .i=1);
        }
        return NULL;
    }
    int test_thread_safety() {

        // create and start test threads (pthread_create both creates and starts)
        pthread_t threads[THREAD_COUNT];
        for (int j = 0; j < THREAD_COUNT; j++) {
            if (pthread_create(&threads[j], NULL, thread_print_loop, (void *)(intptr_t)j) != 0) {
                perror("pthread_create");
                return 1;
            }
        }

        // block the main thread and join the test threads
        // back into main thread when they're done
        for (int j = 0; j < THREAD_COUNT; j++)
            pthread_join(threads[j], NULL);
            // pthread_join:
            // - blocks the calling thread until the thread in its first arg finishes. If the thread has already terminated before you call pthread_join(), then pthread_join() returns immediately
            // - Once the target thread has finished:
            //    - Its return value is stored in its 2nd arg *retval. NOTE: if you don't pass NULL this pointer type must match the return type of the thread's function
            // - The system reclaims the thread’s resources (stack, thread-local storage, etc.)
            // - The thread ID (pthread_t) becomes invalid / reusable
        return 0;
    }
#endif




int main(void) {

    logger = init_log(
        .filepath = "log.txt",
        .output_to_console = true,
        .output_to_logfile = true,
        .clear_old_log = true,
        .prepend_memory_usage=true
    );
    if (!logger) {
        fprintf(stderr, "Failed to initialize logger.\n");
        return -1;
    }
    print(logger, "thread safety test");

    if (test_thread_safety() != 0) {
        fprintf(stderr, "Thread safety test failed\n");
        close_log(logger);
        return 1;
    }
    print(logger, "test complete");

    close_log(logger);
    return 0;
}
