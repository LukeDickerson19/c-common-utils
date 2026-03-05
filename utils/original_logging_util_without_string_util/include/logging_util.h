// #pragma once
#ifndef logging_util_H
#define logging_util_H

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


#define LOGGING_ENABLED true // toggle logging entirely
#define MAX_INDENTS 10 // max number of indents the user can indent a log message
#define MAX_MESSAGE_CHARS 10000 // max number of characters per message, tested w/ value: 500
#define MAX_LINE_CHARS 1000 // max number of characters per line (must be less than MAX_MESSAGE_CHARS), tested w/ value: 150
// #define MAX_MESSAGE_CHARS 500 // FOR TESTING PURPOSES ONLY
// #define MAX_LINE_CHARS 150 // FOR TESTING PURPOSES ONLY


typedef struct {
    char *filepath; // path to the log file
    FILE *console_stream; // FILE* stream to print console output to (e.g., stdout, stderr)
    FILE *file_pointer; // FILE* pointer to the log file
    bool clear_old_log; // flag to clear the log file or not
    bool output_to_console; // flag to print to the console or not
    bool output_to_logfile; // flag to print to the log file or not
    char *console_indent; // what an indent looks like in the console
    char *logfile_indent; // what an indent looks like in the log file
    char *prepend_datetime_fmt; // format specifying datetime to prepend to each line printed
    char *timezone; // timezone to use if prepend_datetime_fmt is not an empty string
    bool prepend_memory_usage; // prepend the memory used and allocated to the program using the logging util

    // variables used for overwrite_prev_msg
    char *prev_console_message;
    size_t prev_console_message_len;
    off_t prev_logfile_start, prev_logfile_end;

    // thread safety struct member
    #if PLATFORM_WINDOWS
        CRITICAL_SECTION mutex;
    #else
        pthread_mutex_t mutex;
    #endif

} Log;

#define DEFAULT_LOG_OPTIONS \
    .filepath = NULL, \
    .console_stream = stdout, .file_pointer = NULL, \
    .output_to_logfile = true, .clear_old_log = true, \
    .output_to_console = true, \
    .console_indent = "|   ", .logfile_indent = "    ", \
    .prepend_datetime_fmt = NULL, .timezone = "UTC", \
    .prepend_memory_usage = false
Log *_init_log(Log *opts);
#define init_log(...) _init_log(&(Log){ DEFAULT_LOG_OPTIONS, ##__VA_ARGS__ })

void close_log(Log *log);

typedef struct {
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
    PrintOptions *opts);

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
#endif // logging_util_H

