#include "log_utils.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#if PLATFORM_WINDOWS
    #include <windows.h>
    #include <psapi.h>
    #include <io.h>
    #include <fcntl.h>
    #define open   _open // _sopen_s
    #define close  _close
    #define write  _write
    #define fileno _fileno
    static int stdout_fd(void) {
        return _fileno(stdout);
    }
    #define STDOUT_FILENO stdout_fd()
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


int _count_lines(const char* str) {
    int count = 0;
    for (const char* p = str; *p; p++) {
        if (*p == '\n') count++;
    }
    return count;
}

void console_clear_previous_message(Log *log) {
    int line_count = _count_lines(log->prev_console_message);

    #if PLATFORM_WINDOWS
        /* ANSI version (preferred on modern Windows) */
        DWORD written;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            for (int i = 0; i < line_count; i++)
                WriteConsoleA(hOut, "\x1b[F\x1b[K", 6, &written, NULL);
            return;
        }
    #endif

    /* POSIX / fallback */
    for (int i = 0; i < line_count; i++)
        write(STDOUT_FILENO, "\033[F\033[K", 6);
        // write(STDOUT_FILENO, "\033[F", 3);  // Cursor up 1 line
        // write(STDOUT_FILENO, "\033[K", 3);  // Clear line
}

char* _fix_utc_format(char* fmt, const char* timezone) {
    /* if "%Z" substring in prepend_datetime_fmt and timezone = "UTC", replace "%Z" with hardcoded "UTC" */

    if (!fmt || !timezone) return fmt;
    if (strcmp(timezone, "UTC") != 0) return fmt;
    char* tz_fmt_ptr = strstr(fmt, "%Z");
    if (!tz_fmt_ptr) return fmt;  // no "%Z" found

    // Allocate new string: prefix + "UTC" + suffix + null terminator
    size_t prefix_len = tz_fmt_ptr - fmt;
    size_t suffix_len = strlen(tz_fmt_ptr + 2); // skip "%Z"
    size_t new_size = prefix_len + 3 + suffix_len + 1;
    char* fixed = malloc(new_size);
    if (!fixed) return fmt;
    memcpy(fixed, fmt, prefix_len); // Copy prefix
    memcpy(fixed + prefix_len, "UTC", 3); // Insert "UTC"
    memcpy(fixed + prefix_len + 3, tz_fmt_ptr + 2, suffix_len); // Copy suffix
    fixed[new_size - 1] = '\0'; // Null-terminate
    // free(fmt); // ownership transfer
    return fixed; // swap pointer, pointer swap
}

void init_log(Log* log) {

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
        int flags = O_CREAT | O_WRONLY; // O_CREAT - create the file if it doesn't exist, O_WRONLY - open for writing
        if (log->clear_old_log) flags |= O_TRUNC;
        log->file_descriptor = open(
            log->filepath,
            flags,
            log->file_permissions // file permissions (ignored on non-Unix systems where defaults will be used instead)
        );
    }

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

    log->prev_console_message = NULL;
    log->prev_logfile_message = NULL;
    log->prev_console_message_len = 0;
    log->prev_logfile_message_len = 0;
    log->prev_logfile_start = -1;
    log->prev_logfile_end   = -1;

}

void close_log(Log *log) {
    if (!log) return;
    if (log->file_descriptor >= 0) close(log->file_descriptor);
    if (log->prev_console_message) free(log->prev_console_message);
    if (log->prev_logfile_message) free(log->prev_logfile_message);
}

char* _get_memory_str(size_t bytes) {
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

int _get_current_time(const char* timezone, char *datetime_str, size_t datetime_cap, char *fmt) {

    // get current time down to microsecond precision
    struct tm tm_info;
    time_t now;
    int micro_seconds;
    #ifdef _WIN32
        FILETIME ft;
        ULARGE_INTEGER uli;
        GetSystemTimePreciseAsFileTime(&ft); // Windows 8+ (high precision)
        uli.LowPart  = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uint64_t t100ns = uli.QuadPart; // FILETIME = 100ns since Jan 1, 1601
        uint64_t us = (t100ns - 116444736000000000ULL) / 10; // Convert to Unix epoch
        now = (time_t)(us / 1000000);
        micro_seconds = (int)(us % 1000000);
    #else
        struct timeval tv;
        if (gettimeofday(&tv, NULL) != 0)
            return -2;
        now = tv.tv_sec;
        micro_seconds = tv.tv_usec;
    #endif

    // get current time in local or UTC timezone
    if (!timezone || strcmp(timezone, "UTC") == 0) {
        #ifdef _WIN32
            if (gmtime_s(&tm_info, &now) != 0) return -2;
        #else
            if (gmtime_r(&now, &tm_info) == NULL) return -2;
        #endif
    } else if (strcmp(timezone, "local") == 0) {
        #ifdef _WIN32
            if (localtime_s(&tm_info, &now) != 0) return -2;
        #else
            if (localtime_r(&now, &tm_info) == NULL) return -2;
        #endif
    } else {
        fprintf(stderr, "Invalid timezone: \"%s\", valid options: \"UTC\", \"local\"\n", timezone);
        return -3;
    }

    // format string of current time
    char *us_ptr = strstr(fmt, "%f");
    char *fmt2 = malloc(datetime_cap);
    if (!fmt2) return -4;
    if (!us_ptr) {
        snprintf(fmt2, datetime_cap, "%s", fmt);
    } else {
        // replace possible "%f" in string fmt with micro_seconds (b/c strftime can't handle microseconds)
        char us_str[7];  // 6 digits + null terminator
        snprintf(us_str, sizeof(us_str), "%06d", micro_seconds);
        size_t prefix_len = us_ptr - fmt;
        size_t suffix_len = strlen(us_ptr + 2); // skip "%f"
        size_t new_size = prefix_len + 6 + suffix_len + 1;
        if (new_size > datetime_cap) return -4; // Buffer too small
        memcpy(fmt2, fmt, prefix_len); // Copy prefix
        memcpy(fmt2 + prefix_len, us_str, 6); // Insert zero padded micro seconds
        memcpy(fmt2 + prefix_len + 6, us_ptr + 2, suffix_len); // Copy suffix
        fmt2[new_size - 1] = '\0'; // Null-terminate
    }
    strftime(datetime_str, datetime_cap, fmt2, &tm_info);
    free(fmt2);
    return 0;
}

int _get_process_memory_usage(char *buf, size_t buf_cap) {
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
        snprintf(buf, buf_cap, "%s used  ", _get_memory_str(bytes));
        return 0;

    fail:
    snprintf(buf, buf_cap, "%s", "Memory read error  ");
    return -1;
}

static size_t snprintf_append(char *dest, size_t dest_cap, size_t *pos, const char *fmt, ...) {
    // Append src string (formatted) to dest buffer with length tracking
    if (!dest || !pos || *pos >= dest_cap) return *pos;

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(dest + *pos, dest_cap - *pos, fmt, args);
    va_end(args);

    if (n < 0) return *pos; // snprintf error, leave pos unchanged
    if ((size_t)n >= dest_cap - *pos) n = dest_cap - *pos; // clamp if truncated
    *pos += (size_t)n;
    return *pos;
}

static void append_inline_truncation_message(
    char *buf,                // buffer to modify
    size_t *pos,              // current length / position in buffer
    const char *truncate_msg, // e.g. " ... log message truncated ..."
    const char *end           // e.g. opts->end ("\n" etc), can be NULL
) {
    // Overwrite end of buffer with a truncation message
    if (!buf || !pos || !end ||*pos == 0) return;

    size_t end_len = end ? strlen(end) : 0;
    size_t msg_len = strlen(truncate_msg) + end_len;

    // Make sure we don't go negative
    size_t write_pos = (*pos > msg_len) ? (*pos - msg_len) : 0;

    // Write truncation message + end at the computed position
    snprintf(buf + write_pos, msg_len + 1, "%s%s", truncate_msg, end);

    // Update current length
    *pos = write_pos + strlen(truncate_msg) + end_len;
}

int _get_formatted_message(
    Log* log,
    const char* message,
    const char* indent,
    char** formatted_message,
    size_t *formatted_message_len,
    PrintOptions *opts
) {

    // validate input args
    if (!message || !formatted_message || !opts) return -1;
    int i = opts->i;
    assert(i >= 0 && i <= MAX_INDENTS);

    // indent buffers
    size_t indent_len = strlen(indent);
    char *total_indent0 = calloc(indent_len * i + 1, 1);
    char *total_indent1 = calloc(indent_len * (i + 1) + 1, 1);
    if (!total_indent0 || !total_indent1)
        goto fail;
    for (int j = 0; j < i; j++)
        memcpy(total_indent0 + j * indent_len, indent, indent_len);
    for (int j = 0; j < i + 1; j++)
        memcpy(total_indent1 + j * indent_len, indent, indent_len);
    const char *total_indent = opts->d ? total_indent1 : total_indent0;

    // Prepend info buffers and variables
    char p_buf1[256] = {0};
    char p_buf2[256] = {0};
    char *p = p_buf1;
    char *scratch = p_buf2;
    size_t p_len = 0;
    const char div_mark = '-';
    const char *mock_indent = " ";
    size_t mock_indent_len = strlen(mock_indent);
    char p0[256] = {0};
    size_t p0_len = 0;

    bool prepend_stuff = (log->prepend_datetime_fmt || log->prepend_memory_usage);
    if (prepend_stuff) {

        // Prepend datetime in specified format
        if (log->prepend_datetime_fmt) {
            char datetime_str[128];
            if (_get_current_time(
                    log->timezone,
                    datetime_str,
                    sizeof(datetime_str),
                    log->prepend_datetime_fmt
                ) != 0)
                goto fail;
            p_len = snprintf(
                scratch,
                sizeof(p_buf2),
                "%s  ", datetime_str);
            PTR_SWAP(p, scratch);
        }

        // Prepend memory usage
        if (log->prepend_memory_usage) {
            char mem_usage_str[256];
            _get_process_memory_usage(mem_usage_str, sizeof(mem_usage_str));
            if (log->prepend_datetime_fmt) {
                p_len = snprintf(
                    scratch,
                    sizeof(p_buf2),
                    "%s%c  %17s", p, div_mark, mem_usage_str);
            } else {
                p_len = snprintf(
                    scratch,
                    sizeof(p_buf2),
                    "%17s", mem_usage_str);
            }
            PTR_SWAP(p, scratch);
        }

        // fill p0 memory with mock indents + div_mark
        for (int j = 0; j < i; j++)
            memcpy(p0 + j * mock_indent_len,
                   mock_indent, mock_indent_len);
        p0_len = mock_indent_len * i;
        p0_len += snprintf(
            p0 + p0_len,
            sizeof(p0) - p0_len,
            "%c ", div_mark);

        // Right-align prepend info before any info prepended to the log message.
        // This is so VS Code's code folding feature continues to work with prepended info.
        p_len = snprintf(
            scratch,
            sizeof(p_buf2),
            "%*s%s%c  ", MAX_INDENTS + 1 - ((int)mock_indent_len * i), "", p, div_mark);
        PTR_SWAP(p, scratch);
    }

    // blank_p = p but w/ prepended info removed, only marks remain
    char *blank_p = NULL;
    if (prepend_stuff) {
        size_t blank_cap = p0_len + p_len + 3;
        blank_p = malloc(blank_cap);
        if (!blank_p)
            goto fail;
        snprintf(
            blank_p,
            blank_cap,
            "%s%*c  ", p0, (int)p_len - 2, div_mark);
    }

    // Init output buffer fmt_msg
    char *fmt_msg = malloc(MAX_MESSAGE_CHARS);
    if (!fmt_msg)
        goto fail;
    size_t fmt_msg_len = 0;

    // Add starting newline if requested
    if (opts->ns) {
        if (prepend_stuff)
            snprintf_append(fmt_msg, MAX_MESSAGE_CHARS, &fmt_msg_len, "%s", p0);
        snprintf_append(fmt_msg, MAX_MESSAGE_CHARS, &fmt_msg_len, "%s\n", total_indent);
    }

    // Format each line in the log message
    const char *line_start = message;
    bool message_truncated = false;
    char line[MAX_LINE_CHARS], fmt_line[MAX_LINE_CHARS];
    do {
        const char *line_end = strchr(line_start, '\n'); // strchr() returns pointer to next occurrence of '\n'
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);
        if (line_len >= MAX_LINE_CHARS) line_len = MAX_LINE_CHARS - 1;
        memcpy(line, line_start, line_len);
        line[line_len] = '\0';
        fmt_line[0] = '\0';
        size_t fmt_line_len = 0;
        if (prepend_stuff) {
            if (line_len == 0)
                snprintf_append(fmt_line, MAX_LINE_CHARS, &fmt_line_len, "%s", blank_p);
            else
                snprintf_append(fmt_line, MAX_LINE_CHARS, &fmt_line_len, "%s%s", p0, p);
        }
        const char *line_indent = line_len == 0 ? total_indent : total_indent0;

        // append line
        snprintf_append(fmt_line, MAX_LINE_CHARS, &fmt_line_len, "%s%s%s", line_indent, line, opts->end);

        // truncate fmt_line if it exceeds MAX_LINE_CHARS
        if (fmt_line_len >= MAX_LINE_CHARS)
            append_inline_truncation_message(
                fmt_line, &fmt_line_len,
                " ... log line truncated ...", opts->end);

        // truncate fmt_msg if it exceeds MAX_MESSAGE_CHARS
        if (fmt_msg_len + fmt_line_len >= MAX_MESSAGE_CHARS) {
            append_inline_truncation_message(
                fmt_msg, &fmt_msg_len,
                " ... log message truncated ...", opts->end);
            message_truncated = true;
            break;
        }

        // Append formatted line to fmt_msg
        snprintf_append(fmt_msg, MAX_MESSAGE_CHARS, &fmt_msg_len, "%s", fmt_line);
        // assert(fmt_msg_len == strlen(fmt_msg)); // FOR TESTING PURPOSES ONLY

        // Move to next line
        line_start = line_end ? line_end + 1 : NULL;
    } while (line_start != NULL);

    // Add ending newline if requested
    if (opts->ne && !message_truncated) {
        if (prepend_stuff)
            snprintf_append(fmt_msg, MAX_MESSAGE_CHARS, &fmt_msg_len, "%s", blank_p);
        snprintf_append(fmt_msg, MAX_MESSAGE_CHARS, &fmt_msg_len, "%s\n", total_indent);
    }

    // Copy final result to formatted_message
    *formatted_message = fmt_msg;
    *formatted_message_len = fmt_msg_len;

    // Free heap memory and return success code
    free(total_indent0);
    free(total_indent1);
    free(blank_p);
    return 0;

    // Free heap memory and return failure code
    fail:
    free(total_indent0);
    free(total_indent1);
    free(blank_p);
    return -1;
}

int _update_prev_message(
    Log* log,
    char* str,
    size_t str_len,
    int output_location // 0 = console, 1 = logfile
) {
    if (!log || !str) return -1;

    char **prev_msg_ptr = NULL;
    size_t *prev_msg_len_ptr = NULL;

    if (output_location == 0) {
        prev_msg_ptr = &log->prev_console_message;
        prev_msg_len_ptr = &log->prev_console_message_len;
    } else if (output_location == 1) {
        prev_msg_ptr = &log->prev_logfile_message;
        prev_msg_len_ptr = &log->prev_logfile_message_len;
    } else {
        return -1;
    }

    // free old message if any
    if (*prev_msg_ptr != NULL) free(*prev_msg_ptr);

    // allocate and copy new message
    size_t len = str_len + 1;
    *prev_msg_ptr = malloc(len);
    if (!*prev_msg_ptr) {
        const char *error_msg = (output_location == 0)
            ? "failed to allocate memory for prev_console_message"
            : "failed to allocate memory for prev_logfile_message";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
        return -1;
    }
    memcpy(*prev_msg_ptr, str, len);
    *prev_msg_len_ptr = str_len;

    return 0;
}

int _log_print(
    Log* log,          // pointer to log struct to use
    const char *msg,   // message to print
    PrintOptions *opts // print optional arguments (see struct PrintOptions for details)
) {

    // Validate arguments
    if (!log || !msg) perror("must pass a Log struct pointer and string message");
    if (opts == NULL) {
        opts = &(PrintOptions){DEFAULT_PRINT_OPTIONS};
    }

    // Print to console
    char *console_str = NULL, *error_msg = NULL;
    size_t console_str_len;
    bool output_to_console = (opts->oc == -1) ? log->output_to_console : opts->oc;
    if (output_to_console) {

        // Move cursor up and clear previous string if user set overwrite_prev_print to true
        if (opts->overwrite_prev_print && log->prev_console_message != NULL)
            console_clear_previous_message(log);
        
        // Format string and print to console
        if(_get_formatted_message(log, msg, log->console_indent, &console_str, &console_str_len, opts) != 0) {
            error_msg = "failed to get formatted console string";
            write(STDOUT_FILENO, error_msg, strlen(error_msg));
            return -1;
        } else {
            write(STDOUT_FILENO, console_str, console_str_len);
            int rc = _update_prev_message(log, console_str, console_str_len, 0);
            free(console_str);
            if (rc != 0) return rc;
        }
    }

    // Print to log file
    char *logfile_str = NULL;
    size_t logfile_str_len;
    bool output_to_logfile = (opts->of == -1) ? log->output_to_logfile : opts->of;
    if (output_to_logfile && log->file_descriptor != -1) {

        // Format string for log file
        if(_get_formatted_message(log, msg, log->logfile_indent, &logfile_str, &logfile_str_len, opts) != 0) {
            error_msg = "failed to get formatted log file string";
            write(STDOUT_FILENO, error_msg, strlen(error_msg));
            return -1;
        }

        // Clear previous message in log file if user set overwrite_prev_print to true
        // and print formatted message to log file
        if (opts->overwrite_prev_print && log->prev_logfile_message != NULL) {
            off_t start = log->prev_logfile_start;
            lseek(log->file_descriptor, start, SEEK_SET);
            write(log->file_descriptor, logfile_str, logfile_str_len);
            off_t new_end = start + logfile_str_len;

            // truncate if new message is shorter
            if (new_end < log->prev_logfile_end)
                ftruncate(log->file_descriptor, new_end);
            log->prev_logfile_end = new_end;

        } else {
            // Normal append
            log->prev_logfile_start = lseek(log->file_descriptor, 0, SEEK_CUR);
            write(log->file_descriptor, logfile_str, logfile_str_len);
            log->prev_logfile_end = lseek(log->file_descriptor, 0, SEEK_CUR);
        }
        fsync(log->file_descriptor);

        // then free the logfile message memory, and update prev_logfile_message
        int rc = _update_prev_message(log, logfile_str, logfile_str_len, 1);
        free(logfile_str);
        if (rc != 0) return rc;
    }

    return 0;
}
