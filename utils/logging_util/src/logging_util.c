#include "logging_util.h"
#include "string_util.h"
#include "time_util.h"

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



// macro used to swap pointers to avoid extra character copies and speed up performance
#define PTR_SWAP(a, b) do { char *tmp = (a); (a) = (b); (b) = tmp; } while (0)



static char* _fix_utc_format(char* format, const char* timezone) {
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

    // TODO:
    // .prepend_elapsed_time = false, \
    // .unix_start_time = 0, \
    // .start_time_microseconds = 0, \

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
    if ((size_t)n >= dest_cap - *pos) n = dest_cap - *pos; // clamp if truncated
    *pos += (size_t)n;
    return *pos;
}

static void _append_inline_truncation_message(
    char *buf,                // buffer to modify
    size_t *buf_len,          // current length / position in buffer
    const char *truncate_msg, // e.g. " ... log message truncated ..."
    const char *end           // e.g. opts->end ("\n" etc), can be NULL
) {
    // Overwrite end of buffer with a truncation message
    if (!buf || !buf_len || !end || *buf_len == 0) return;

    size_t end_len = end ? strlen(end) : 0;
    size_t msg_len = strlen(truncate_msg) + end_len;

    // Calculate write_pos, leaving room for msg_len + null terminator
    size_t write_pos = (*buf_len > msg_len + 1) ? (*buf_len - (msg_len + 1)) : 0; // +1 for null terminator

    // Write the message, with space for the null terminator
    snprintf(buf + write_pos, msg_len + 1, "%s%s", truncate_msg, end);

    // Update current length (excluding null terminator)
    *buf_len = write_pos + msg_len;
}

static int _get_indented_message(
    const char *message,
    char *indent,
    int max_msg_chars, int max_ln_chars,
    bool prepend_stuff,
    char *p0, char *p, char *blank_p,
    char div_mark,
    char **formatted_message,
    size_t *formatted_message_len,
    PrintOptions *opts
) {

    // Create indent buffers
    size_t indent_len = strlen(indent);
    int i = opts->i;
    char *total_indent1 = calloc(indent_len * i + 1, 1);
    char *total_indent2 = calloc(indent_len * (i + 1) + 1, 1);
    if (!total_indent1 || !total_indent2) goto fail;
    for (int j = 0; j < i;     j++) memcpy(total_indent1 + j * indent_len, indent, indent_len);
    for (int j = 0; j < i + 1; j++) memcpy(total_indent2 + j * indent_len, indent, indent_len);
    const char* total_indent3 = opts->d ? total_indent2 : total_indent1;

    // Init output buffer fmt_msg
    char *fmt_msg = malloc(max_msg_chars);
    if (!fmt_msg)
        goto fail;
    size_t fmt_msg_len = 0;

    // Add starting newline if requested
    if (opts->ns) {
        if (prepend_stuff)
            _snprintf_append(fmt_msg, max_msg_chars, &fmt_msg_len, "%s", p0);
        _snprintf_append(fmt_msg, max_msg_chars, &fmt_msg_len, "%s\n", total_indent3);
    }

    // Format each line in the log message
    const char *line_start = message;
    bool message_truncated = false;
    char *line = malloc(max_ln_chars);
    char *fmt_line = malloc(max_ln_chars);
    do {
        const char *line_end = strchr(line_start, '\n'); // strchr() returns pointer to next occurrence of '\n'
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);
        if (line_len >= max_ln_chars) line_len = max_ln_chars - 1;
        memcpy(line, line_start, line_len);
        line[line_len] = '\0';
        fmt_line[0] = '\0';
        size_t fmt_line_len = 0;
        if (prepend_stuff) {
            if (line_len == 0)
                _snprintf_append(fmt_line, max_ln_chars, &fmt_line_len, "%s", blank_p);
            else
                _snprintf_append(fmt_line, max_ln_chars, &fmt_line_len, "%s%c%s", p0, div_mark, p);
        }
        const char *line_indent = line_len == 0 ? total_indent3 : total_indent1;

        // append line
        _snprintf_append(fmt_line, max_ln_chars, &fmt_line_len, "%s%s%s", line_indent, line, opts->end);

        // truncate fmt_line if it exceeds max_ln_chars
        if (fmt_line_len >= max_ln_chars)
            _append_inline_truncation_message(
                fmt_line, &fmt_line_len,
                " ... log line truncated ...", opts->end);

        // truncate fmt_msg if it exceeds max_msg_chars
        if (fmt_msg_len + fmt_line_len >= max_msg_chars) {
            _append_inline_truncation_message(
                fmt_msg, &fmt_msg_len,
                " ... log message truncated ...", opts->end);
            message_truncated = true;
            break;
        }

        // Append formatted line to fmt_msg
        _snprintf_append(fmt_msg, max_msg_chars, &fmt_msg_len, "%s", fmt_line);

        // Move to next line
        line_start = line_end ? line_end + 1 : NULL;
    } while (line_start != NULL);

    // Add ending newline if requested
    if (opts->ne && !message_truncated) {
        if (prepend_stuff)
            _snprintf_append(fmt_msg, max_msg_chars, &fmt_msg_len, "%s", blank_p);
        _snprintf_append(fmt_msg, max_msg_chars, &fmt_msg_len, "%s\n", total_indent3);
    }

    // Copy final result to formatted_message
    *formatted_message = fmt_msg;
    *formatted_message_len = fmt_msg_len;

    // Free heap memory and return success code
    free(total_indent1);
    free(total_indent2);
    free(line);
    free(fmt_line);
    return 0;

    // Free heap memory and return failure code
    fail:
    free(total_indent1);
    free(total_indent2);
    return -1;
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
    // p = prepended info text
    // p0 = mock indents: If info is prepended to each line, mock indents are tiny indents before the prepended info. They exist so VS Code's code folding feature continues to work when there's prepended info, and the prepended info remains veritically alligned.
    char p_buf[512] = {0}, p0_buf[128] = {0};
    char *p = p_buf, *p0 = p0_buf;
    size_t p_len = 0, p0_len = 0;
    const char div_mark = '-';
    const char *mock_indent = " ";
    size_t mock_indent_len = strlen(mock_indent);
    char s_buf[256] = {0};
    char *scratch = s_buf;
    bool prepend_stuff = \
        log->prepend_datetime_fmt || \
        log->prepend_elapsed_time || \
        log->prepend_memory_usage;
    if (prepend_stuff) {

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
            p_len = snprintf(
                scratch,
                sizeof(s_buf),
                "%s  ", datetime_str);
            PTR_SWAP(p, scratch);
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
                p_len = snprintf(
                    scratch,
                    sizeof(s_buf),
                    "%s%c  %s  ", p, div_mark, elapsed_time_str);
            } else {
                p_len = snprintf(
                    scratch,
                    sizeof(s_buf),
                    "%s%s  ", p, elapsed_time_str);
            }
            PTR_SWAP(p, scratch);
        }

        // Prepend memory usage
        if (log->prepend_memory_usage) {
            char mem_usage_str[256];
            _get_process_memory_usage(mem_usage_str, sizeof(mem_usage_str));
            if (log->prepend_datetime_fmt || log->prepend_elapsed_time) {
                p_len = snprintf(
                    scratch,
                    sizeof(s_buf),
                    "%s%c  %17s", p, div_mark, mem_usage_str);
            } else {
                p_len = snprintf(
                    scratch,
                    sizeof(s_buf),
                    "%17s", mem_usage_str);
            }
            PTR_SWAP(p, scratch);
        }

        // fill p0 memory with mock indents + div_mark
        for (int j = 0; j < i; j++)
            memcpy(p0 + j * mock_indent_len,
                   mock_indent, mock_indent_len);
        p0_len = mock_indent_len * i;

        // Right-align prepend info before any info prepended to the log message.
        // This is so VS Code's code folding feature continues to work with prepended info.
        p_len = snprintf(
            scratch,
            sizeof(s_buf),
            "%*s%s%c  ", log->max_indents + 1 - (int)p0_len, "", p, div_mark);
        PTR_SWAP(p, scratch);
    }

    // blank_p is the same as p but w/ prepend info removed, only marks remain
    char *blank_p = NULL;
    if (prepend_stuff) {
        size_t blank_cap = p0_len + p_len + 3;
        blank_p = malloc(blank_cap);
        if (!blank_p)
            goto fail;
        snprintf(
            blank_p,
            blank_cap,
            "%s%c%*c  ", p0, div_mark, (int)p_len - 3, div_mark);
    }

    if (create_console_output)
        _get_indented_message(
            message,
            log->console_indent,
            log->max_message_chars, log->max_line_chars,
            prepend_stuff,
            p0, p, blank_p,
            div_mark,
            console_msg,
            console_msg_len,
            opts
        );

    if (create_logfile_output)
        _get_indented_message(
            message,
            log->logfile_indent,
            log->max_message_chars, log->max_line_chars,
            prepend_stuff,
            p0, p, blank_p,
            div_mark,
            logfile_msg,
            logfile_msg_len,
            opts
        );

    // Free heap memory and return success code
    free(blank_p);
    return 0;

    // Free heap memory and return failure code
    fail:
    free(blank_p);
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
