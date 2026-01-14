// #pragma once
#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#if defined(_WIN32)
    #define PLATFORM_WINDOWS 1
    #include <io.h>
    typedef long off_t;  // Windows fallback for file offsets
#else
    #define PLATFORM_WINDOWS 0
    #include <sys/types.h> // for off_t
    #if defined(__APPLE__)
        // #include <mach/mach.h> // INCLUDED IN log_utils.c
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
    bool prepend_memory_usage; // prepend the memory used and allocated to the python program

    // variables used for overwrite_prev_print
    char *prev_console_message, *prev_logfile_message;
    size_t prev_console_message_len, prev_logfile_message_len;
    off_t prev_logfile_start, prev_logfile_end;

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
#define INIT_LOG(...) _init_log(&(Log){ DEFAULT_LOG_OPTIONS, ##__VA_ARGS__ })

void close_log(Log *log);

typedef struct {
    int  i;  // number of indents to put in front of the string, defaults to 0
    bool ns; // print a new line in before the string, defaults to false
    bool ne; // print a new line in after the string, defaults to false
    int  oc; // output to console, defaults to -1, which uses the Log struct's output_to_console bool, else oc (0 = false, 1 = true)
    int  of; // output to logfile, defaults to -1, which uses the Log struct's output_to_logfile bool, else of (0 = false, 1 = true)
    bool d;  // draw a line on the blank line before or after the string, defaults to false
    bool overwrite_prev_print; // overwrite previous print statement in console, does nothing in logfile, defaults to false
    char *end; // last character(s) to print at the end of the string, optional arg - defaults to '\n'
    // char *console_str; // string printed to console
    // char *logfile_str; // string printed to logfile // TODO: return these strings from _log_print if the user passes a string buffer, throw error if buffer is too small
} PrintOptions;

#define DEFAULT_PRINT_OPTIONS \
    .i = 0, \
    .ns = false, \
    .ne = false, \
    .oc = -1, \
    .of = -1, \
    .d = false, \
    .overwrite_prev_print = false, \
    .end = "\n" //
    // .console_str = NULL, \
    // .logfile_str = NULL

// macro used to format log messages
// NOTE: _buffer variable's life time is the entire program cause its "static"
// however each time FMT(...) is called this same memory is overwritten
#define FMT(...) \
    ({ \
        static char _buffer[MAX_MESSAGE_CHARS]; \
        snprintf(_buffer, sizeof(_buffer), __VA_ARGS__); \
        _buffer; \
    })

int _log_print(
    Log *log,
    const char *msg,
    PrintOptions *opts);

// this macro exists to simulate optional args in C
#define PRINT(logger, msg, ...) if (LOGGING_ENABLED) _log_print((logger), (msg), &(PrintOptions){ DEFAULT_PRINT_OPTIONS, ##__VA_ARGS__})
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.



#ifdef __cplusplus
}
#endif
#endif // LOG_UTILS_H

