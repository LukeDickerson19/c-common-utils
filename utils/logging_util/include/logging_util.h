// #pragma once
#ifndef LOGGING_UTIL_H
#define LOGGING_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>   // for malloc, free, exit
#if defined(_WIN32)
    #define PLATFORM_WINDOWS 1
    #include <windows.h>
    #include <io.h>
    typedef long off_t;  // Windows fallback for file offsets
#else
    #define PLATFORM_WINDOWS 0
    #include <pthread.h>
    #include <sys/types.h> // for off_t
    #if defined(__APPLE__)
        // #include <mach/mach.h> // INCLUDED IN logging_util.c
    #elif defined(__linux__) || defined(__ANDROID__)
        #include <unistd.h>  // for POSIX off_t
    #endif
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct Log {

    bool enabled; // toggle logging entirely

    bool output_to_logfile; // flag to print to the log file or not
    bool clear_old_log; // flag to clear the log file or not
    char *filepath; // path to the log file
    FILE *file_pointer; // FILE* pointer to the log file
    char *logfile_indent; // what an indent looks like in the log file

    bool output_to_console; // flag to print to the console or not
    FILE *console_stream; // FILE* stream to print console output to (e.g., stdout, stderr)
    char *console_indent; // what an indent looks like in the console

    char *prepend_datetime_fmt; // format specifying datetime to prepend to each line printed
    char *timezone; // timezone to use if prepend_datetime_fmt is not an empty string
    bool prepend_elapsed_time; // flag to prepend the time elapsed since the the Log's unix_start_time
    int unix_start_time; // unix start time used for prepending elapsed time, defaults to time when init_log() is called
    int start_time_microseconds; // microsecond component of unix start time
    bool prepend_memory_usage; // prepend the memory used and allocated to the program using the logging util
    int max_indents; // max number of indents the user can indent a log message // NOTE: max_indents effects mini indents when prepending time or memory info, keep it as small as you estimate the max number of indents you'll use
    int max_message_chars; // max number of characters per message
    int max_line_chars; // max number of characters per line

    // variables used for overwrite_prev_msg
    char *prev_console_message;
    size_t prev_console_message_len;
    off_t prev_logfile_start, prev_logfile_end;

    // thread safety mutex
    #if PLATFORM_WINDOWS
        CRITICAL_SECTION mutex;
    #else
        pthread_mutex_t mutex;
    #endif

} Log;

#define DEFAULT_LOG_OPTIONS \
    .enabled = true, \
    .output_to_logfile = false, \
    .clear_old_log = false, \
    .filepath = NULL, \
    .file_pointer = NULL, \
    .logfile_indent = "    ", \
    .output_to_console = true, \
    .console_stream = stdout, \
    .console_indent = "|   ", \
    .prepend_datetime_fmt = NULL, \
    .timezone = "UTC", \
    .prepend_elapsed_time = false, \
    .unix_start_time = 0, \
    .start_time_microseconds = 0, \
    .prepend_memory_usage = false, \
    .max_indents = 10, \
    .max_message_chars = 10000, \
    .max_line_chars = 1000
Log *_init_log(Log *opts);
#define init_log(...) _init_log(&(Log){ DEFAULT_LOG_OPTIONS, ##__VA_ARGS__ })


void close_log(
    Log *log
);


typedef struct PrintOptions {
    int  i;  // number of indents to put in front of the string, defaults to 0
    bool ns; // print a new line in before the string, defaults to false
    bool ne; // print a new line in after the string, defaults to false
    int  oc; // output to console, defaults to -1, which uses the Log struct's output_to_console bool, else oc (0 = false, 1 = true)
    int  of; // output to logfile, defaults to -1, which uses the Log struct's output_to_logfile bool, else of (0 = false, 1 = true)
    bool d;  // draw a line on the blank line before or after the string, defaults to false
    bool overwrite_prev_msg; // overwrite previous printed message in console and logfile
    char *end; // last character(s) to print at the end of the string, defaults to "\n"
    char **console_str; // pointer to string printed to console
    char **logfile_str; // pointer to string printed to logfile
} PrintOptions;

#define DEFAULT_PRINT_OPTIONS \
    .i = 0, \
    .ns = false, \
    .ne = false, \
    .oc = -1, \
    .of = -1, \
    .d = false, \
    .overwrite_prev_msg = false, \
    .end = "\n", \
    .console_str = NULL, \
    .logfile_str = NULL

int _log_print(
    Log *log,
    const char *msg,
    PrintOptions *opts
);

// this macro exists to simulate optional args in C
#define print(logger, msg, ...) if (LOGGING_ENABLED) _log_print((logger), (msg), &(PrintOptions){ DEFAULT_PRINT_OPTIONS, ##__VA_ARGS__})
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.

// macro used to format log messages
// NOTE: _buf is a stack-allocated array, so each thread calling the macro
// gets its own independent buffer on its own stack
#define fmt(...) ({ \
    char _buf[MAX_MESSAGE_CHARS]; \
    snprintf(_buf, MAX_MESSAGE_CHARS, __VA_ARGS__); \
    _buf; \
})


#ifdef __cplusplus
}
#endif
#endif // LOGGING_UTIL_H

