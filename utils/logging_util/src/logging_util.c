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


/////////////////////// time functions /////////////////////


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

Log *_log_init(
    Log *opts
) {

    // init default and user specified log options
    Log *log = malloc(sizeof(Log));
    if (!log) return NULL; // allocation failed
    if (!opts)
        opts = &(Log){ DEFAULT_LOG_OPTIONS }; // in case user calls _log_init without log_init macro
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

    // only output to log file if filepath is provided
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

    // fix weird timezone bug:
    // if "%Z" substring in prepend_datetime_fmt and
    // timezone = "UTC", replace "%Z" with hardcoded "UTC"
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
    // these variables are reused each log message to avoid frequent
    // allocations and deallocations on the heap
    log->console_msg        = str("", .allocation_procedure=MEM_FIXED, .cap=log->max_message_len * 4);
    log->prev_console_msg   = str("", .allocation_procedure=MEM_FIXED, .cap=log->max_message_len * 4);
    log->logfile_msg        = str("", .allocation_procedure=MEM_FIXED, .cap=log->max_message_len * 4);
    log->logfile_msg_start  = 0;
    log->logfile_msg_end    = 0;

    // init mutex
    #if PLATFORM_WINDOWS
        InitializeCriticalSection(&log->mutex);
    #else
        pthread_mutex_init(&log->mutex, NULL);
    #endif

    return log;
}


void log_close(
    Log **log_ptr
) {
    if (!log_ptr || !*log_ptr) return;
    Log *log = *log_ptr;

    // free log file pointer
    if (log->file_pointer != NULL) fclose(log->file_pointer);

    // str_free skips over any NULL String pointers
    str_free(
        &(log->logfile_indent),
        &(log->console_indent),
        &(log->console_msg),
        &(log->prev_console_msg),
        &(log->logfile_msg)
    );

    // destroy mutex
    #if PLATFORM_WINDOWS
        DeleteCriticalSection(&log->mutex);
    #else
        pthread_mutex_destroy(&log->mutex);
    #endif

    // free struct and NULL log_close() caller's log pointer
    free(log);
    *log_ptr = NULL;
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


static void _clear_previous_console_message(
    Log *log
) {
    int line_count = _count_lines(log->prev_console_msg->text);

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
    char *prepend_info,
    size_t prepend_info_len,
    const String *indent,
    size_t max_message_len,
    size_t max_line_len,
    String *formatted_message,
    PrintOptions *opts
) {

    // Create indent buffers
    size_t indent_len = indent->len;
    size_t i = opts->i;
    size_t len1 = indent_len * i;
    size_t len2 = indent_len * (i + 1);
    char total_indent1[len1 + 1];
    char total_indent2[len2 + 1];
    for (size_t j = 0; j < i; j++)
        memcpy(total_indent1 + j * indent_len, indent->text, indent_len);
    for (size_t j = 0; j < i + 1; j++)
        memcpy(total_indent2 + j * indent_len, indent->text, indent_len);
    total_indent1[len1] = '\0';
    total_indent2[len2] = '\0';
    const char* total_indent3 = opts->d ? total_indent2 : total_indent1;

    // Init tmp buffer for fmt()
    char tmp[max_line_len * 4];

    // Add starting newline if requested
    if (opts->ns)
        str_append(
            formatted_message,
            fmt(tmp, "%s%s\n", prepend_info, total_indent3)
        );

    // Format each line in the log message
    const char *line_byte_start = message;
    bool message_truncated = false;
    do {
        const char *line_byte_end = strchr(line_byte_start, '\n'); // strchr() returns a pointer to next occurrence of '\n'
        size_t line_byte_len = line_byte_end ? (size_t)(line_byte_end - line_byte_start) : strlen(line_byte_start);

        // Append prepended info, indents, and line
        const char *line_indent = line_byte_len == 0 ? total_indent3 : total_indent1;
        const size_t rune_len_before = formatted_message->len;
        str_append(
            formatted_message,
            fmt(tmp, "%s%s%.*s%s",
                prepend_info,
                line_indent,
                line_byte_len,
                line_byte_start,
                opts->end
            )
        );

        // Truncate formatted_message and break if its too long
        if (formatted_message->len >= max_message_len) {
            str_slice(
                formatted_message,
                0, max_message_len - MESSAGE_TRUNCATION_MSG_LEN
            );
            str_append(
                formatted_message,
                MESSAGE_TRUNCATION_MSG
            );
            message_truncated = true;
            break;
        }
        
        // Truncate line if its too long
        const size_t line_rune_len = formatted_message->len - rune_len_before;
        if (line_rune_len >= max_line_len) {
            str_slice(
                formatted_message,
                0, rune_len_before + max_line_len - LINE_TRUNCATION_MSG_LEN
            );
            str_append(
                formatted_message,
                LINE_TRUNCATION_MSG
            );
        }

        // Move to next line
        line_byte_start = line_byte_end ? line_byte_end + 1 : NULL;
    } while (line_byte_start != NULL);

    // Add ending newline if requested
    if (opts->ne && !message_truncated)
        str_append(
            formatted_message,
            fmt(tmp, "%s%s\n", prepend_info, total_indent3)
        );

    // Return success code
    return 0;
}


static int _get_formatted_messages(
    Log* log,
    const char* message,
    bool output_to_console,
    bool output_to_logfile,
    PrintOptions *opts
) {

    // validate input args
    if (!message || !opts || !log) return -1;
    int i = opts->i;
    if (i < 0 || i > log->max_indents) return -1;

    // Prepend info if requested
    char prepend_info[log->max_line_len];
    prepend_info[0] = '\0'; // Initialize to empty string for when not prepending anything
    size_t prepend_info_len = 0;
    if (log->prepend_datetime_fmt || \
        log->prepend_elapsed_time || \
        log->prepend_memory_usage) {

        // append mock indents, then div_mark, then pad the end so the whole thing is log->max_indents long
        // If info is prepended to each line, mock indents are tiny one space indents before the prepended info.
        // They exist so VS Code's code folding feature continues to work when there's prepended info, and the prepended info remains veritically alligned.
        const char div_mark = '-';
        fmt_append(
            prepend_info, sizeof(prepend_info), &prepend_info_len,
            "%*s%c%*s", i, "", div_mark, log->max_indents - i, ""
        );

        // get current time if needed
        int64_t unix_seconds;
        int32_t microseconds;
        if (log->prepend_datetime_fmt || log->prepend_elapsed_time) {
            if (get_current_unix_time(&unix_seconds, &microseconds) != 0) {
                fprintf(stderr, "LOG ERROR: failed to get current time\n");
                return -1;
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
                return -1;
            }
            fmt_append(
                prepend_info, sizeof(prepend_info), &prepend_info_len,
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
                return -1;
            }
            char elapsed_time_str[128];
            if (format_elapsed_time(
                elapsed_sec,
                elapsed_usec,
                elapsed_time_str,
                sizeof(elapsed_time_str)) != 0) {

                fprintf(stderr, "LOG ERROR: failed to format elapsed time\n");
                return -1;
            }
            if (log->prepend_datetime_fmt) {
                fmt_append(
                    prepend_info, sizeof(prepend_info), &prepend_info_len,
                    "%c  ", div_mark
                );
            }
            fmt_append(
                prepend_info, sizeof(prepend_info), &prepend_info_len,
                "%s  ", elapsed_time_str
            );
        }

        // Prepend memory usage
        if (log->prepend_memory_usage) {
            char mem_usage_str[256];
            _get_process_memory_usage(mem_usage_str, sizeof(mem_usage_str));
            if (log->prepend_datetime_fmt || log->prepend_elapsed_time) {
                fmt_append(
                    prepend_info, sizeof(prepend_info), &prepend_info_len,
                    "%c  ", div_mark
                );
            }
            fmt_append(
                prepend_info, sizeof(prepend_info), &prepend_info_len,
                "%17s", mem_usage_str
            );
        }

        // append a final div mark plus some spacing
        fmt_append(
            prepend_info, sizeof(prepend_info), &prepend_info_len,
            "%c  ", div_mark
        );
    }

    if (output_to_console) {
        str_clear(log->console_msg);
        _get_indented_message(
            message,
            prepend_info,
            prepend_info_len,
            log->console_indent,
            log->max_message_len,
            log->max_line_len,
            log->console_msg,
            opts
        );
    }

    if (output_to_logfile) {
        str_clear(log->logfile_msg);
        _get_indented_message(
            message,
            prepend_info,
            prepend_info_len,
            log->logfile_indent,
            log->max_message_len,
            log->max_line_len,
            log->logfile_msg,
            opts
        );
    }

    return 0;
}


int _log_print_unlocked(
    Log* log,
    const char *msg,
    PrintOptions *opts
) {
    if (!opts) opts = &(PrintOptions){DEFAULT_PRINT_OPTIONS};
    int return_code = 0;

    // Use optional opts->oc/of arg(s) if specified, else default to log struct's setting
    bool output_to_console = (opts->oc == -1) ? log->output_to_console : opts->oc;
    bool output_to_logfile = (opts->of == -1) ? log->output_to_logfile : opts->of;

    // Get formatted log message for console and/or log file
    if (output_to_console || output_to_logfile) {
        if (_get_formatted_messages(
                log, msg,
                output_to_console,
                output_to_logfile,
                opts
            ) != 0) {

            fprintf(stderr, "LOG ERROR: failed to format message\n");
            return -1;
        }
    }

    // Write to console
    if (output_to_console) {

        // Move cursor up and clear previous message if user set overwrite_prev_msg to true
        if (opts->overwrite_prev_msg && !str_is_empty(log->prev_console_msg))
            _clear_previous_console_message(log);

        // Print formatted string to console
        fwrite(log->console_msg->text, 1, log->console_msg->bytes, log->console_stream);
        // NOTE: using fwrite() instead of write() even though its buffered because it
        // works with FILE* streams, is fully cross-platform, and we can disable buffering
        // with setvbuf(_IONBF). fprintf() automatically handles \0-terminated strings,
        // Use fwrite to respect console_msg->len if binary content

        // Overwrite the old prev_console_msg with the new console_msg
        int rc = str_overwrite(log->prev_console_msg, log->console_msg->text);
        if (rc != 0)
            return_code = rc;
    }

    // Write to log file
    if (output_to_logfile && log->file_pointer != NULL) {

        // Clear previous message in log file if user set overwrite_prev_msg to true
        long write_pos;
        if (opts->overwrite_prev_msg && log->logfile_msg_end > log->logfile_msg_start) {
            write_pos = log->logfile_msg_start;
            fseek(log->file_pointer, write_pos, SEEK_SET);
        } else {
            write_pos = ftell(log->file_pointer);
            log->logfile_msg_start = write_pos;
        }

        // Print formatted message to log file
        fwrite(log->logfile_msg->text, 1, log->logfile_msg->bytes, log->file_pointer);

        // Update message file start/end position
        long new_end = write_pos + log->logfile_msg->bytes;
        if (opts->overwrite_prev_msg) // Truncate if overwriting shorter content
            ftruncate(fileno(log->file_pointer), new_end);
        log->logfile_msg_end = new_end;
    }

    return return_code;
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


