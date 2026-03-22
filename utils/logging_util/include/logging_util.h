// #pragma once
#ifndef LOGGING_UTIL_H
#define LOGGING_UTIL_H

#include "string_util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>   // for malloc, free, exit
#include <stdint.h>  // Required for int64_t, int32_t
#include <stddef.h>  // Required for size_t
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


/////////////// logging functions //////////////


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
    int64_t unix_start_time; // unix start time used for prepending elapsed time, defaults to time when log_init() is called
    int32_t start_time_microseconds; // microsecond component of unix start time
    bool prepend_memory_usage; // prepend the memory used and allocated to the program using the logging util
    Buffer *p_buf; // short char array thats passed to the string_util fmt_append() function to provide a temporary buffer for formatting char arrays. Its set to a size of 4 * log->max_line_len for worst case utf-8 4 byte characters. Its malloced and free'd with the log struct instead of print() to improve print() function's performance.
    size_t max_indents; // max number of indents the user can indent a log message // NOTE: max_indents effects mini indents when prepending time or memory info, keep it as small as you estimate the max number of indents you'll use
    char *max_console_indentation; // max console indentation (heap allocated on log_init with console_indent and max_indents)
    char *max_logfile_indentation; // max logfile indentation (heap allocated on log_init with logfile_indent and max_indents)
    Buffer *i_buf; // short char array buffer thats passed to the string_util fmt() function to provide a temporary buffer for formatting char arrays. Its set to a size of 4 * log->max_line_len for worst case utf-8 4 byte characters. Its malloced and free'd with the log struct instead of print() to improve print() function's performance.
    size_t max_message_len; // max number of runes (aka UTF-8 code points) per message
    size_t max_line_len; // max number of runes (aka UTF-8 code points) per line, NOTE: if max_line_len is too large it can cause a stack overflow error, recommend at max 4096.

    // latest and 2nd latest console message printed
    String *console_msg;

    // latest logfile message printed, and its 
    String *logfile_msg;
    off_t logfile_msg_start, logfile_msg_end;

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
    .max_message_len = 8192, \
    .max_line_len = 1024
Log *_log_init(Log *opts);
#define log_init(...) _log_init(&(Log){ DEFAULT_LOG_OPTIONS, ##__VA_ARGS__ })


void log_close(
    Log **log_ptr
);


#define LINE_TRUNCATION_MSG " ... log line truncated ...\n"
#define MESSAGE_TRUNCATION_MSG " ... log message truncated ...\n"
#define LINE_TRUNCATION_MSG_LEN sizeof(LINE_TRUNCATION_MSG) - 1
#define MESSAGE_TRUNCATION_MSG_LEN sizeof(MESSAGE_TRUNCATION_MSG) - 1


typedef struct PrintOptions {
    int  i;  // number of indents to put in front of the string, defaults to 0
    bool ns; // print a new line in before the string, defaults to false
    bool ne; // print a new line in after the string, defaults to false
    int  oc; // output to console, defaults to -1, which uses the Log struct's output_to_console bool, else oc (0 = false, 1 = true)
    int  of; // output to logfile, defaults to -1, which uses the Log struct's output_to_logfile bool, else of (0 = false, 1 = true)
    bool d;  // draw a line on the blank line before or after the string, defaults to false
    bool overwrite_prev_msg; // overwrite previous printed message in console and logfile
    char *end; // last character(s) to print at the end of the string, defaults to "\n"
} PrintOptions;

#define DEFAULT_PRINT_OPTIONS \
    .i = 0, \
    .ns = false, \
    .ne = false, \
    .oc = -1, \
    .of = -1, \
    .d = false, \
    .overwrite_prev_msg = false, \
    .end = "\n"

int _log_print(
    Log *log,
    const char *msg,
    PrintOptions *opts
);

// this macro exists to simulate optional args in C
#define print(logger, msg, ...) _log_print((logger), (msg), &(PrintOptions){ DEFAULT_PRINT_OPTIONS, ##__VA_ARGS__})
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.


// getter functions for the char array of the latest message printed to the console and logfile
static inline char *get_latest_console_msg(Log *log) { return log->console_msg->text; }
static inline char *get_latest_logfile_msg(Log *log) { return log->logfile_msg->text; }
// NOTE: "inline" alone tells the compiler each translation unit (aka c code file) can have its own copy, but doesn't guarantee one gets emitted. "static inline" ensures each translation unit that includes the header gets its own private copy, which is the standard pattern for inline functions in headers.
// A "translation unit" is one .c file after the preprocessor has run on it — meaning after all #includes have been pasted in and all macros expanded. It's the single chunk of code the compiler sees and compiles into one .o object file.


///////////////// time functions ///////////////


/** sleep_microseconds() pauses the program for a specified number of microseconds
 *   - microseconds: number of microseconds to pause
 */
void sleep_microseconds(
    int64_t microseconds
);


/**
 * Sets the Log's start time
 * - log: pointer to Log struct
 * - new_unix_start_time / new_start_time_microseconds: optional pointers to override start time
 *   If NULL, use get_current_unix_time() to set current time
 */
void set_start_time(
    Log *log,
    const int64_t *new_unix_start_time,
    const int32_t *new_start_time_microseconds
);


////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif // LOGGING_UTIL_H

