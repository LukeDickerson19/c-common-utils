#include "time_util.h"

#include <time.h>          // time_t, struct tm, time(), localtime_r(), gmtime_r(), strftime(), gettimeofday()
#include <stdio.h>         // fprintf(), snprintf()
#include <string.h>        // strcmp(), strstr(), memcpy(), strncat(), strlen()
#include <stdlib.h>        // malloc(), free()

#ifdef _WIN32
    #include <windows.h>   // FILETIME, GetSystemTimePreciseAsFileTime()
    #define gmtime_r(t, tm)    (gmtime_s((tm), (t)) == 0 ? (tm) : NULL)
    #define localtime_r(t, tm) (localtime_s((tm), (t)) == 0 ? (tm) : NULL)
    #define snprintf _snprintf
// #elif defined(__APPLE__)
//     #include <mach/mach.h>
// #elif defined(__linux__) || defined(__ANDROID__)
//     #include <unistd.h>
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
        if (!gmtime_r(&sec, &tm_info)) return -2;
    } else if (strcmp(timezone, "local") == 0) {
        if (!localtime_r(&sec, &tm_info)) return -3;
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
    if (strftime(datetime_str, datetime_str_capacity, expanded_fmt, &tm_info) == 0)
        return -7;
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


