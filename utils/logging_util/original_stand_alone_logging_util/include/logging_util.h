// #pragma once
#ifndef LOGGING_UTIL_H
#define LOGGING_UTIL_H

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
    int64_t unix_start_time; // unix start time used for prepending elapsed time, defaults to time when init_log() is called
    int32_t start_time_microseconds; // microsecond component of unix start time
    bool prepend_memory_usage; // prepend the memory used and allocated to the program using the logging util
    size_t max_indents; // max number of indents the user can indent a log message // NOTE: max_indents effects mini indents when prepending time or memory info, keep it as small as you estimate the max number of indents you'll use
    size_t max_message_chars; // max number of characters per message
    size_t max_line_chars; // max number of characters per line, NOTE: if max_line_chars is too large it can cause a stack overflow error, recommend at max 4096.

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
    .max_message_chars = 8192, \
    .max_line_chars = 1024
Log *_init_log(Log *opts);
#define init_log(...) _init_log(&(Log){ DEFAULT_LOG_OPTIONS, ##__VA_ARGS__ })


void close_log(
    Log *log
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
#define print(logger, msg, ...) _log_print((logger), (msg), &(PrintOptions){ DEFAULT_PRINT_OPTIONS, ##__VA_ARGS__})
// NOTE: __VA_ARGS__ override default print options because when they're later in the struct initialization
// The prepended "##" characters is a GNU extension that removes the comma if __VA_ARGS__ is empty. This is widely supported but not part of the C standard.

/**
 * fmt() is a convienience macro used to format char arrays. It requires passing a pre-created buffer managed by the caller, so use multiple buffers if nesting fmt() calls, or calling fmt() multiple times on one line so they don't interfere with each other.
 * Example usage:
 *     char *buf1[128];
 *     char *buf2[buf_size]; // Example buffer sized at runtime. C99 allows pre-defined stack buffers to have runtime determined sizes because you can use Variable Length Arrays (VLAs). function's arg
 *     printf("%s %s\n", fmt(buf1, "A"), fmt(buf2, "B"));
 * 
 * @param buf      pointer to a char array
 * @param ...      printf-style format string followed by any values to substitute
 * @return         pointer to the formatted string, buf
 */
#define fmt(buf, ...) ( \
    snprintf(buf, sizeof(buf), __VA_ARGS__), \
    buf \
)
/* NOTE:
    int snprintf(char *str, size_t size, const char *format, ...);
        Args:
            str     Pointer to the destination buffer.
            size    Maximum number of bytes to write to the buffer including the null terminator.
            format  Format string (same syntax as printf).
            ...     Values to substitute into the format string.
*/


/**
 * fmt_append() appends the src string (plus formatting) to dst buffer, and updates the *pos pointer to the new end of the char array, and the null terminator.
 * 
 * @param dst      pointer to the destination buffer
 * @param dst_cap  size of the destination buffer
 * @param pos      position in dst to append to or overwrite
 * @param src      source text to write into dst
 * @param ...      printf-style string formattingx values to substitute into src
 * @return         number of bytes written
 */
size_t fmt_append(
    char *dst,
    size_t dst_cap,
    size_t *pos,
    const char *src,
    ...
);


///////////////// time functions ///////////////


/* get_current_unix_time() retrieves the current Unix time with microsecond precision
    - unix_seconds:  output pointer for seconds since Unix epoch (1970-01-01 00:00:00 UTC)
    - microseconds:  output pointer for microseconds component (0–999999)
*/
int get_current_unix_time(
    int64_t *unix_seconds,
    int32_t *microseconds
);


/* format_datetime_str() returns the current datetime as a formatted string (supports microsecond level resolution with "%f" format)
    - timezone: "local" or "UTC", defaults to UTC
    - format:   datetime formats are based on strftime:
            https://man7.org/linux/man-pages/man3/strftime.3.html
            plus %f format for microseconds like in python:
            https://strftime.org/
*/
int format_datetime_str(
    int64_t unix_seconds,
    int32_t microseconds,
    const char *timezone,
    const char *format,
    char *out,
    size_t datetime_str_capacity
);


/* get_elapsed_time() computes elapsed time between two timestamps with microsecond precision
    - start_sec / start_usec: starting time (seconds + microseconds)
    - end_sec / end_usec:     ending time (seconds + microseconds)
    - elapsed_sec:            output pointer for elapsed seconds
    - elapsed_usec:           output pointer for remaining microseconds (0–999999)
*/
int get_elapsed_time(
    int64_t start_sec,
    int32_t start_usec,
    int64_t end_sec,
    int32_t end_usec,
    int32_t *elapsed_sec,
    int32_t *elapsed_usec
);


/* format_elapsed_time() formats elapsed time as HH:MM:SS.ffffff
    - elapsed_sec:  elapsed seconds
    - elapsed_usec: elapsed microseconds
    - elapsed_time_str: output buffer for formatted string
    - elapsed_time_str_cap: capacity of output buffer
*/
int format_elapsed_time(
    int32_t elapsed_sec,
    int32_t elapsed_usec,
    char *elapsed_time_str,
    size_t elapsed_time_str_cap
);


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

