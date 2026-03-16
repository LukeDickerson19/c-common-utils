#include "logging_util.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <wchar.h>
#if PLATFORM_WINDOWS
    #include <windows.h>
    #include <psapi.h> // for PROCESS_MEMORY_COUNTERS and GetProcessMemoryInfo
    #include <io.h>
    #include <fcntl.h>
    #define open   _open // _sopen_s
    #define close  _close
    #define write  _write
    #define fileno _fileno
    static int stdout_fd(void) {
        return _fileno(stdout);
    }
    // #define STDOUT_FILENO stdout_fd() // unneeded after switching from write() to fwrite()
    static void enable_virtual_terminal_processing(void) {
        // Enable ANSI on Windows. Windows does not enable ANSI by default.
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return;
        DWORD mode = 0;
        if (!GetConsoleMode(hOut, &mode)) return;
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, mode);
    }
    #define lseek _lseek
    #define ftruncate _chsize
    #define fsync _commit
#else
    #include <sys/time.h>
    #if defined(__APPLE__)
        #include <mach/mach.h>
    #elif defined(__linux__) || defined(__ANDROID__)
        // #include <stdio.h>   // for snprintf // ALREADY INCLUDED IN HEADER
        // #include <unistd.h>  // for sysconf and off_t // ALREADY INCLUDED IN HEADER
    #endif
#endif



/////////////////////// time functions /////////////////////

#include <time.h>          // time_t, struct tm, time(), localtime_r(), gmtime_r(), strftime(), gettimeofday()
#include <stdio.h>         // fprintf(), snprintf()
#include <string.h>        // strcmp(), strstr(), memcpy(), strncat(), strlen()
#include <stdlib.h>        // malloc(), free(), setenv()

#ifdef _WIN32
    #include <windows.h>   // FILETIME, GetSystemTimePreciseAsFileTime()
    #include <psapi.h> // for PROCESS_MEMORY_COUNTERS and GetProcessMemoryInfo
// #elif defined(__APPLE__)
//     #include <mach/mach.h>
// #elif defined(__linux__) || defined(__ANDROID__)
//     #include <unistd.h>
    #define gmtime_r(t, tm)   gmtime_s((tm), (t))
    #define localtime_r(t, tm) localtime_s((tm), (t))
    #define snprintf _snprintf
#else
    #include <sys/time.h>   // struct timeval, gettimeofday()
    #include <unistd.h>     // usleep()
#endif


int get_current_unix_time(
    int64_t *unix_seconds,
    int32_t *microseconds
) {
    if (!unix_seconds || !microseconds) return -1;

    #ifdef _WIN32
        FILETIME ft;
        ULARGE_INTEGER uli;
        GetSystemTimePreciseAsFileTime(&ft); // Windows 8+ (high precision)
        uli.LowPart  = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uint64_t t100ns = uli.QuadPart; // 100ns since Jan 1, 1601. 100 ns ticks are the highest resolution windows provides
        uint64_t us = (t100ns - 116444736000000000ULL) / 10; // Convert to the unix epoch (seconds since 1970-01-01 00:00:00 UTC)
        *unix_seconds = (int64_t)(us / 1000000ULL);
        *microseconds = (int32_t)(us % 1000000ULL);
    #else
        struct timeval tv;
        if (gettimeofday(&tv, NULL) != 0) // gettimeofday() -> unix seconds since 1970 new years
            return -2;

        *unix_seconds = (int64_t)tv.tv_sec; 
        *microseconds = (int32_t)tv.tv_usec; 
    #endif

    return 0;    
}


int format_datetime_str(
    int64_t unix_seconds,
    int32_t microseconds,
    const char *timezone,
    const char *format,
    char *datetime_str,
    size_t datetime_str_capacity
) {
    if (!format || !datetime_str) return -1;

    // get current datetime in local or UTC timezone
    struct tm tm_info;
    time_t sec = (time_t)unix_seconds;
    if (!timezone || strcmp(timezone, "UTC") == 0) {
        #ifdef _WIN32
            if (gmtime_s(&tm_info, &sec) != 0) return -2;
        #else
            if (!gmtime_r(&sec, &tm_info)) return -2;
        #endif
    } else if (strcmp(timezone, "local") == 0) {
        #ifdef _WIN32
            if (localtime_s(&tm_info, &sec) != 0) return -3;
        #else
            if (!localtime_r(&sec, &tm_info)) return -3;
        #endif
    } else {
        fprintf(stderr, "Invalid timezone: \"%s\", valid options: \"UTC\", \"local\"\n", timezone);
        return -4;
    }

    // format time into string with microsecond resolution
    const char *us_ptr = strstr(format, "%f");
    char *expanded_fmt = malloc(datetime_str_capacity);
    if (!expanded_fmt) return -5;
    if (!us_ptr) {
        snprintf(expanded_fmt, datetime_str_capacity, "%s", format);
    } else {
        // replace possible "%f" in string format with micro_seconds (b/c strftime can't handle microseconds)
        char us_str[7];
        snprintf(us_str, sizeof(us_str), "%06d", microseconds);
        size_t prefix_len = us_ptr - format;
        size_t suffix_len = strlen(us_ptr + 2); // skip "%f"
        size_t total_len  = prefix_len + 6 + suffix_len + 1;
        if (total_len > datetime_str_capacity) {
            free(expanded_fmt);
            return -6; // Buffer too small
        }
        memcpy(expanded_fmt, format, prefix_len); // Copy prefix
        memcpy(expanded_fmt + prefix_len, us_str, 6); // Insert zero padded micro seconds
        memcpy(expanded_fmt + prefix_len + 6, us_ptr + 2, suffix_len); // Copy suffix
        expanded_fmt[total_len - 1] = '\0'; // Null-terminate
    }
    strftime(datetime_str, datetime_str_capacity, expanded_fmt, &tm_info);
    free(expanded_fmt);
    return 0;
}


int get_elapsed_time(
    int64_t start_sec,
    int32_t start_usec,
    int64_t end_sec,
    int32_t end_usec,
    int32_t *elapsed_sec,
    int32_t *elapsed_usec
) {
    if (!elapsed_sec || !elapsed_usec) return -1;
    *elapsed_sec  = (int32_t)(end_sec - start_sec);
    *elapsed_usec = end_usec - start_usec;

    // handle microsecond underflow
    if (*elapsed_usec < 0) {
        *elapsed_usec += 1000000;
        *elapsed_sec  -= 1;
    }
    return 0;
}


int format_elapsed_time(
    int32_t elapsed_sec,
    int32_t elapsed_usec,
    char *elapsed_time_str,
    size_t elapsed_time_str_cap
) {
    if (!elapsed_time_str || elapsed_time_str_cap < 16) return -1;
    if (elapsed_usec < 0 || elapsed_usec >= 1000000) return -2;

    int hours   =  elapsed_sec / 3600;
    int minutes = (elapsed_sec % 3600) / 60;
    int seconds =  elapsed_sec % 60;

    // formatted so HH can exceed 24 for long durations
    int n = snprintf(
        elapsed_time_str,
        elapsed_time_str_cap,
        "%02d:%02d:%02d.%06d",
        hours, minutes, seconds, elapsed_usec
    );

    // Size elapsed_time_str of bounds
    if (n < 0 || (size_t)n >= elapsed_time_str_cap) return -3;

    return 0;
}


void sleep_microseconds(
    int64_t microseconds
) {
    #ifdef _WIN32
        Sleep((DWORD)(microseconds / 1000)); // windows max precision is milliseconds
    #else
        struct timespec ts;
        ts.tv_sec = microseconds / 1000000;
        ts.tv_nsec = (microseconds % 1000000) * 1000;
        nanosleep(&ts, NULL);
    #endif
}


static char* _fix_utc_format(
    char* format,
    const char* timezone
) {
    /* if "%Z" substring in prepend_datetime_fmt and timezone = "UTC", replace "%Z" with hardcoded "UTC" */

    if (!format || !timezone) return format;
    if (strcmp(timezone, "UTC") != 0) return format;
    char* tz_fmt_ptr = strstr(format, "%Z");
    if (!tz_fmt_ptr) return format;  // no "%Z" found

    // Allocate new string: prefix + "UTC" + suffix + null terminator
    size_t prefix_len = tz_fmt_ptr - format;
    size_t suffix_len = strlen(tz_fmt_ptr + 2); // skip "%Z"
    size_t new_size = prefix_len + 3 + suffix_len + 1;
    char* fixed = malloc(new_size);
    if (!fixed) return format;
    memcpy(fixed, format, prefix_len); // Copy prefix
    memcpy(fixed + prefix_len, "UTC", 3); // Insert "UTC"
    memcpy(fixed + prefix_len + 3, tz_fmt_ptr + 2, suffix_len); // Copy suffix
    fixed[new_size - 1] = '\0'; // Null-terminate
    // free(format); // ownership transfer
    return fixed; // swap pointer, pointer swap
}


void set_start_time(
    Log *log,
    const int64_t *new_unix_start_time,
    const int32_t *new_start_time_microseconds
) {
    if (!log) return;

    if (new_unix_start_time && new_start_time_microseconds) {
        log->unix_start_time = *new_unix_start_time;
        log->start_time_microseconds = *new_start_time_microseconds;
    } else {
        int64_t sec;
        int32_t usec;
        if (get_current_unix_time(&sec, &usec) == 0) {
            log->unix_start_time = (int)sec;
            log->start_time_microseconds = usec;
        } else {
            log->unix_start_time = 0;
            log->start_time_microseconds = 0;
        }
    }
}


//////////////////////// logging functions /////////////////

Log *_init_log(
    Log *opts
) {

    // init default and user specified log options
    Log *log = malloc(sizeof(Log));
    if (!log) return NULL; // allocation failed
    if (!opts)
        opts = &(Log){ DEFAULT_LOG_OPTIONS }; // in case user calls _init_log without init_log macro
    *log = *opts;

    // return early if logging is disabled
    if (!log->enabled) return log;

    // enable ansi on windows
    #if PLATFORM_WINDOWS
        static int ansi_enabled = 0;
        if (!ansi_enabled && log->output_to_console) {
            enable_virtual_terminal_processing();
            ansi_enabled = 1;
        }
    #endif

    log->output_to_logfile = (log->filepath != NULL) ? log->output_to_logfile : false;

    // create logfile if it doesn't exist, and clear it if user specified to do so
    if (log->filepath != NULL) {
        log->file_pointer = fopen(
            log->filepath,
            log->clear_old_log ? "w" : "a" // w = truncate, a = append
        );
    }

    // disable buffering for immedate output
    if (log->output_to_console)
        setvbuf(log->console_stream, NULL, _IONBF, 0); // disable buffering for console_stream
    if (log->output_to_logfile && log->file_pointer != NULL)
        setvbuf(log->file_pointer, NULL, _IONBF, 0); // disable buffering for logfile

    // fix weird timezone bug: if "%Z" substring in prepend_datetime_fmt
    // and timezone = "UTC", replace "%Z" with hardcoded "UTC"
    if (log->prepend_datetime_fmt && log->timezone) {
        char* fixed_fmt = _fix_utc_format(log->prepend_datetime_fmt, log->timezone);
        // only replace pointer if a new string was returned
        if (fixed_fmt != log->prepend_datetime_fmt) {
            // free old string if it was heap-allocated
            // NOTE: be careful not to free literals
            // optional: track if fmt was heap-allocated; otherwise just assign
            log->prepend_datetime_fmt = fixed_fmt;
        }
    }
    // TODO: assert valid prepend_datetime_fmt
    // TODO: assert valid timezone

    // initialize log start time to current time
    set_start_time(log, NULL, NULL);

    // init overwrite_prev_msg variables
    log->prev_console_message = NULL;
    log->prev_console_message_len = 0;
    log->prev_logfile_start = 0;
    log->prev_logfile_end   = 0;

    // init mutex
    #if PLATFORM_WINDOWS
        InitializeCriticalSection(&log->mutex);
    #else
        pthread_mutex_init(&log->mutex, NULL);
    #endif

    return log;
}


void close_log(
    Log *log
) {

    // free struct members
    if (!log) return;
    if (log->file_pointer != NULL) fclose(log->file_pointer);
    if (log->prev_console_message) free(log->prev_console_message);

    // destroy mutex
    #if PLATFORM_WINDOWS
        DeleteCriticalSection(&log->mutex);
    #else
        pthread_mutex_destroy(&log->mutex);
    #endif

    // free struct
    free(log);
}


static int _count_lines(
    const char* str
) {
    int count = 0;
    for (const char* p = str; *p; p++) {
        if (*p == '\n') count++;
    }
    return count;
}


static void _console_clear_previous_message(
    Log *log
) {
    int line_count = _count_lines(log->prev_console_message);

    // // NOTE: not needed after switching from write() to fwrite(), maybe delete one day
    // #if PLATFORM_WINDOWS
    //     /* ANSI version (preferred on modern Windows) */
    //     DWORD written;
    //     HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    //     if (hOut != INVALID_HANDLE_VALUE) {
    //         for (int i = 0; i < line_count; i++)
    //             WriteConsoleA(hOut, "\x1b[F\x1b[K", 6, &written, NULL);
    //         return;
    //     }
    // #endif

    // Using C standard fwrite for cross-platform console output
    const char *seq = "\033[F\033[K"; // cursor up + clear line
    for (int i = 0; i < line_count; i++) {
        fwrite(seq, 1, 6, log->console_stream);
        fflush(log->console_stream);  // ensure immediate effect
    }
}


static char* _get_memory_str(
    size_t bytes
) {
    // converts the int number of bytes to a string with appropriate units
    static char buffer[64];
    const char* units[] = {"bytes", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    const int num_units = sizeof(units) / sizeof(units[0]);
    double b = (double)bytes;
    int index = 0;
    while (b >= 1024 && index < num_units - 1) {
        b /= 1024.0;
        index++;
    }
    if (index == 0) {
        if (bytes == 1) {
            snprintf(buffer, sizeof(buffer), "1 byte");
        } else {
            snprintf(buffer, sizeof(buffer), "%zu bytes", bytes);
        }
    } else {
        snprintf(buffer, sizeof(buffer), "%.4f %s", b, units[index]);
    }
    return buffer;
}


static int _get_process_memory_usage(
    char *buf,
    size_t buf_cap
) {
    if (!buf || buf_cap == 0)
        return -1;
    size_t bytes = 0;

    #if defined(_WIN32)
        PROCESS_MEMORY_COUNTERS pmc;
        if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
            goto fail;
        bytes = (size_t)pmc.WorkingSetSize;

    #elif defined(__APPLE__)
        mach_task_basic_info info;
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(
                mach_task_self(),
                MACH_TASK_BASIC_INFO,
                (task_info_t)&info,
                &count
            ) != KERN_SUCCESS)
            goto fail;
        bytes = (size_t)info.resident_size;

    #elif defined(__linux__) || defined(__ANDROID__)
        long rss_pages = 0;
        FILE *f = fopen("/proc/self/statm", "r");
        if (!f)
            goto fail;
        if (fscanf(f, "%*s %ld", &rss_pages) != 1) {
            fclose(f);
            goto fail;
        }
        fclose(f);
        long page_size = sysconf(_SC_PAGESIZE);
        if (page_size <= 0)
            goto fail;
        bytes = (size_t)rss_pages * (size_t)page_size;

    #else
        goto fail;

    #endif

    snprintf(buf, buf_cap, "%14s used  ", _get_memory_str(bytes));
    return 0;

    fail:
    snprintf(buf, buf_cap, "%s", "Memory read error  ");
    return -1;
}


static size_t _snprintf_append(
    char *dest,
    size_t dest_cap,
    size_t *pos,
    const char *format,
    ...
) {
    // Append src string (formatted) to dest buffer with length tracking
    if (!dest || !pos || *pos >= dest_cap) return *pos;

    va_list args;
    va_start(args, format);
    int n = vsnprintf(dest + *pos, dest_cap - *pos, format, args);
    va_end(args);

    if (n < 0) return *pos; // snprintf error, leave pos unchanged
    size_t written = (size_t)n;
    if (written >= dest_cap - *pos) {
        written = dest_cap - *pos - 1;  // leave room for null
    }
    *pos += written;

    // null-terminate updated string
    if (*pos < dest_cap)
        dest[*pos] = '\0';
    else
        dest[dest_cap-1] = '\0';
    return written;
}


static void _append_inline_truncation_message(
    char *buf,
    size_t buf_cap,
    size_t *buf_len,
    const char *truncate_msg
) {
    if (!buf || !buf_len || !truncate_msg) return;

    size_t msg_len = strlen(truncate_msg);

    size_t remaining = (buf_cap > *buf_len) ? buf_cap - *buf_len : 0;
    if (remaining == 0) return;

    size_t write_len = msg_len;
    if (write_len >= remaining)
        write_len = remaining - 1;  // leave room for null

    memcpy(buf + *buf_len, truncate_msg, write_len);
    *buf_len += write_len;

    buf[*buf_len] = '\0';
}


static int _get_indented_message(
    const char *message,
    const char *indent,
    size_t max_msg_chars, size_t max_ln_chars,
    char *p_buf,
    char **formatted_message, size_t *formatted_message_len,
    PrintOptions *opts
) {

    // Init output buffer fmt_msg
    size_t msg_buf_len = max_msg_chars + MESSAGE_TRUNCATION_MSG_LEN + 1;
    char *fmt_msg = malloc(msg_buf_len);
    if (!fmt_msg) return -1;
    size_t fmt_msg_len = 0;

    // Create indent buffers
    size_t indent_len = strlen(indent);
    size_t i = opts->i;
    size_t len1 = indent_len * i;
    size_t len2 = indent_len * (i + 1);
    char total_indent1[len1 + 1];
    char total_indent2[len2 + 1];
    for (size_t j = 0; j < i; j++)
        memcpy(total_indent1 + j * indent_len, indent, indent_len);
    for (size_t j = 0; j < i + 1; j++)
        memcpy(total_indent2 + j * indent_len, indent, indent_len);
    total_indent1[len1] = '\0';
    total_indent2[len2] = '\0';
    const char* total_indent3 = opts->d ? total_indent2 : total_indent1;

    // Add starting newline if requested
    if (opts->ns)
        _snprintf_append(
            fmt_msg, msg_buf_len, &fmt_msg_len,
            "%s%s\n", p_buf, total_indent3);

    // Format each line in the log message
    const char *line_start = message;
    bool message_truncated = false;
    do {
        const char *line_end = strchr(line_start, '\n'); // strchr() returns pointer to next occurrence of '\n'
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);

        // Append prepended info, indents, and line,
        // and truncate line if its too long.
        const char *line_indent = line_len == 0 ? total_indent3 : total_indent1;
        if (line_len >= max_ln_chars) {
            _snprintf_append(
                fmt_msg, msg_buf_len, &fmt_msg_len,
                "%s%s%.*s",
                p_buf,
                line_indent,
                (int)max_ln_chars,
                line_start
            );
            _append_inline_truncation_message(
                fmt_msg,
                msg_buf_len,
                &fmt_msg_len,
                LINE_TRUNCATION_MSG
            );

        } else {
            _snprintf_append(
                fmt_msg, msg_buf_len, &fmt_msg_len,
                "%s%s%.*s%s",
                p_buf,
                line_indent,
                (int)line_len,
                line_start,
                opts->end
            );
        }

        // truncate fmt_msg and break if fmt_line exceeds max_msg_chars
        if (fmt_msg_len >= max_msg_chars) {
            fmt_msg_len = max_msg_chars;
            _append_inline_truncation_message(
                fmt_msg,
                msg_buf_len,
                &fmt_msg_len,
                MESSAGE_TRUNCATION_MSG
            );
            message_truncated = true;
            break;
        }

        // Move to next line
        line_start = line_end ? line_end + 1 : NULL;
    } while (line_start != NULL);

    // Add ending newline if requested
    if (opts->ne && !message_truncated)
        _snprintf_append(
            fmt_msg, msg_buf_len, &fmt_msg_len,
            "%s%s\n", p_buf, total_indent3);

    // Copy final result to formatted_message
    *formatted_message = fmt_msg;
    *formatted_message_len = fmt_msg_len;

    // Return success code
    return 0;
}


static int _get_formatted_messages(
    Log* log,
    const char* message,
    bool create_console_output, char** console_msg, size_t *console_msg_len,
    bool create_logfile_output, char** logfile_msg, size_t *logfile_msg_len,
    PrintOptions *opts
) {

    // validate input args
    if (!message || !opts || !log) return -1;
    int i = opts->i;
    if (i < 0 || i > log->max_indents) return -1;

    // Prepend info if requested
    char p_buf[log->max_line_chars];
    p_buf[0] = '\0'; // Initialize to empty string for case when prepend_stuff is false
    size_t p_len = 0;
    const char div_mark = '-';
    bool prepend_stuff = \
        log->prepend_datetime_fmt || \
        log->prepend_elapsed_time || \
        log->prepend_memory_usage;
    if (prepend_stuff) {

        // append mock indents, then div_mark, then pad the end so the whole thing is log->max_indents long
        // If info is prepended to each line, mock indents are tiny one space indents before the prepended info.
        // They exist so VS Code's code folding feature continues to work when there's prepended info, and the prepended info remains veritically alligned.
        _snprintf_append(
            p_buf, sizeof(p_buf), &p_len,
            "%*s%c%*s", i, "", div_mark, log->max_indents - i, ""
        );

        // get current time if needed
        int64_t unix_seconds;
        int32_t microseconds;
        if (log->prepend_datetime_fmt || log->prepend_elapsed_time) {
            if (get_current_unix_time(&unix_seconds, &microseconds) != 0) {
                fprintf(stderr, "LOG ERROR: failed to get current time\n");
                goto fail;
            }
        }

        // Prepend datetime in specified format
        if (log->prepend_datetime_fmt) {
            char datetime_str[128];
            if (format_datetime_str(
                unix_seconds,
                microseconds,
                log->timezone,
                log->prepend_datetime_fmt,
                datetime_str,
                sizeof(datetime_str)) != 0) {

                fprintf(stderr, "LOG ERROR: failed to format datetime\n");
                goto fail;
            }
            _snprintf_append(
                p_buf, sizeof(p_buf), &p_len,
                "%s  ", datetime_str
            );
        }

        // Prepend elapsed time since log's start time in HH:MM:SS.ffffff format
        if (log->prepend_elapsed_time) {
            int32_t elapsed_sec, elapsed_usec;
            if (get_elapsed_time(
                log->unix_start_time,
                log->start_time_microseconds,
                unix_seconds,
                microseconds,
                &elapsed_sec,
                &elapsed_usec) != 0) {

                fprintf(stderr, "LOG ERROR: failed to get elapsed time\n");
                goto fail;
            }
            char elapsed_time_str[128];
            if (format_elapsed_time(
                elapsed_sec,
                elapsed_usec,
                elapsed_time_str,
                sizeof(elapsed_time_str)) != 0) {

                fprintf(stderr, "LOG ERROR: failed to format elapsed time\n");
                goto fail;
            }
            if (log->prepend_datetime_fmt) {
                _snprintf_append(
                    p_buf, sizeof(p_buf), &p_len,
                    "%c  ", div_mark
                );
            }
            _snprintf_append(
                p_buf, sizeof(p_buf), &p_len,
                "%s  ", elapsed_time_str
            );
        }

        // Prepend memory usage
        if (log->prepend_memory_usage) {
            char mem_usage_str[256];
            _get_process_memory_usage(mem_usage_str, sizeof(mem_usage_str));
            if (log->prepend_datetime_fmt || log->prepend_elapsed_time) {
                _snprintf_append(
                    p_buf, sizeof(p_buf), &p_len,
                    "%c  ", div_mark
                );
            }
            _snprintf_append(
                p_buf, sizeof(p_buf), &p_len,
                "%17s", mem_usage_str
            );
        }

        // append a final div mark plus some spacing
        _snprintf_append(
            p_buf, sizeof(p_buf), &p_len,
            "%c  ", div_mark
        );
    }

    if (create_console_output)
        _get_indented_message(
            message,
            log->console_indent,
            log->max_message_chars, log->max_line_chars,
            p_buf,
            console_msg, console_msg_len,
            opts
        );

    if (create_logfile_output)
        _get_indented_message(
            message,
            log->logfile_indent,
            log->max_message_chars, log->max_line_chars,
            p_buf,
            logfile_msg, logfile_msg_len,
            opts
        );

    return 0;

    fail:
    return -1;
}


static int _update_prev_message(
    Log* log,
    const char* str,
    size_t str_len
) {
    if (!log || !str) return -1;

    char **prev_msg_ptr = &log->prev_console_message;
    size_t *prev_msg_len_ptr = &log->prev_console_message_len;

    // free old message if any
    if (*prev_msg_ptr != NULL) {
        free(*prev_msg_ptr);
        *prev_msg_ptr = NULL;
    }

    // allocate and copy new message (include null terminator)
    size_t len = str_len + 1;
    *prev_msg_ptr = (char*)malloc(len);
    if (!*prev_msg_ptr) {
        const char *error_msg = "failed to allocate memory for prev_console_message\n";
        fprintf(stderr, "%s", error_msg);
        return -1;
    }
    memcpy(*prev_msg_ptr, str, str_len);
    (*prev_msg_ptr)[str_len] = '\0';  // null-terminate
    *prev_msg_len_ptr = str_len;

    return 0;
}


int _log_print_unlocked(
    Log* log,
    const char *msg,
    PrintOptions *opts
) {
    if (!opts) opts = &(PrintOptions){DEFAULT_PRINT_OPTIONS};

    // Use optional opts->oc/of arg(s) if specified, else default to log struct's setting
    bool output_to_console = (opts->oc == -1) ? log->output_to_console : opts->oc;
    bool output_to_logfile = (opts->of == -1) ? log->output_to_logfile : opts->of;

    // Create formatted console and/or logfile strings if needed (printed or returned)
    bool create_console_output = output_to_console || opts->console_str != NULL;
    bool create_logfile_output = output_to_logfile || opts->logfile_str != NULL;
    char *console_str = NULL; size_t console_str_len = 0;
    char *logfile_str = NULL; size_t logfile_str_len = 0;
    if (create_console_output || create_logfile_output) {
        if (_get_formatted_messages(
                log, msg,
                create_console_output, &console_str, &console_str_len,
                create_logfile_output, &logfile_str, &logfile_str_len,
                opts
            ) != 0) {

            fprintf(stderr, "LOG ERROR: failed to format message\n");
            return -1;
        }
    }

    if (console_str != NULL) {

        // Print to console
        if (output_to_console) {

            // Move cursor up and clear previous string if user set overwrite_prev_msg to true
            if (opts->overwrite_prev_msg && log->prev_console_message != NULL)
                _console_clear_previous_message(log);

            // Print formatted string to console
            fwrite(console_str, 1, console_str_len, log->console_stream);
            // NOTES: using fwrite() instead of write() even though its buffered because it works with FILE* streams,
            // is fully cross-platform, and we can disable buffering with setvbuf(_IONBF).
            // fprintf() automatically handles \0-terminated strings, Use fwrite to respect console_str_len if binary content

            // Update previous message tracking
            int rc = _update_prev_message(log, console_str, console_str_len);
            if (rc != 0) {
                free(console_str);
                free(logfile_str);
                return rc;
            }
        }

        // Return console_str to user if requested
        if (opts->console_str) {
            *opts->console_str = console_str; // user now owns memory
        } else {
            free(console_str);
        }
    }

    if (logfile_str != NULL) {

        // Print to log file
        if (output_to_logfile && log->file_pointer != NULL) {

            // Clear previous message in log file if user set overwrite_prev_msg to true
            long write_pos;
            if (opts->overwrite_prev_msg && log->prev_logfile_end > log->prev_logfile_start) {
                write_pos = log->prev_logfile_start;
                fseek(log->file_pointer, write_pos, SEEK_SET);
            } else {
                write_pos = ftell(log->file_pointer);
                log->prev_logfile_start = write_pos;
            }

            // Print formatted message to log file
            fwrite(logfile_str, 1, logfile_str_len, log->file_pointer);

            // Update previous message
            long new_end = write_pos + logfile_str_len;
            if (opts->overwrite_prev_msg) // Truncate if overwriting shorter content
                ftruncate(fileno(log->file_pointer), new_end);
            log->prev_logfile_end = new_end;
        }

        // Return logfile_str to user if requested it, else free its memory
        if (opts->logfile_str) {
            *opts->logfile_str = logfile_str;
        } else {
            free(logfile_str);
        }
    }

    return 0;
}


int _log_print(
    Log* log,       // pointer to log struct to use
    const char *msg,   // message to print
    PrintOptions *opts // optional print arguments
) {
    if (!log || !msg) {
        perror("must pass a Log struct pointer and string message");
        return -1;
    }

    if (!log->enabled) return 0;

    // Lock mutex
    #if PLATFORM_WINDOWS
        EnterCriticalSection(&log->mutex);
    #else
        pthread_mutex_lock(&log->mutex);
    #endif

    // Call _log_print_unlocked()
    int rc = _log_print_unlocked(log, msg, opts);

    // Unlock mutex
    #if PLATFORM_WINDOWS
        LeaveCriticalSection(&log->mutex);
    #else
        pthread_mutex_unlock(&log->mutex);
    #endif

    return rc;
}


////////////////////////////////////////////////////////////


