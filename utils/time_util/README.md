# time-util

#### DESCRIPTION

> Time utility written in C.

##### Main struct and string init function:
```c
typedef struct {
    char   *text;
    size_t  len;
    size_t  cap;
} String;

String str(const char *fmt, ...);
```

##### Features:
> - microsecond precision
> - Functions:
>   - get_current_unix_time
>   - format_datetime_str
>   - get_elapsed_time
>   - format_elapsed_time
>
> - see [include/time_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/time_util/include/time_util.h) for function definitions and descriptions.


#### USAGE
Below is a copy of [tests/full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/time_util/tests/full_example.c).
```c
#include "time_util.h"
#include <stdint.h> // int64_t, int32_t
#include <stdio.h> // stderr

int main(void) {

    // set start time for testing elapsed time
    int64_t start_sec;
    int32_t start_usec;
    if (get_current_unix_time(&start_sec, &start_usec) != 0) {
        fprintf(stderr, "get_current_unix_time() failed\n");
        return 1;
    }

    // format start time
    //     datetime formats are based on strftime:
    //         https://man7.org/linux/man-pages/man3/strftime.3.html
    //     plus %f format for microseconds like in python:
    //         https://strftime.org/
    const char *datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z";
    const char *timezone = "UTC";
    // const char *timezone = "local";
    char start_time_str[128];
    if (format_datetime_str(
            start_sec,
            start_usec,
            timezone,
            datetime_fmt,
            start_time_str,
            sizeof(start_time_str)
        ) != 0) {
        fprintf(stderr, "format_datetime_str() failed\n");
        return 1;
    }
    printf("start time:    %s\n", start_time_str);

    printf("simulating 3 seconds of work...\n");
    fflush(stdout); // print immediately
    sleep_microseconds(3000000);

    // get end time for testing elapsed time
    int64_t end_sec;
    int32_t end_usec;
    if (get_current_unix_time(&end_sec, &end_usec) != 0) {
        fprintf(stderr, "get_current_unix_time() failed\n");
        return 1;
    }

    // compute elapsed time
    int32_t elapsed_sec;
    int32_t elapsed_usec;
    if (get_elapsed_time(
            start_sec,
            start_usec,
            end_sec,
            end_usec,
            &elapsed_sec,
            &elapsed_usec
        ) != 0) {
        fprintf(stderr, "get_elapsed_time() failed\n");
        return 1;
    }

    // format elapsed duration
    char elapsed_time_str[32];
    if (format_elapsed_time(
            elapsed_sec,
            elapsed_usec,
            elapsed_time_str,
            sizeof(elapsed_time_str)
        ) != 0) {
        fprintf(stderr, "format_elapsed_time() failed\n");
        return 1;
    }
    printf("Elapsed time:  %s\n", elapsed_time_str);

    // format end time
    char end_time_str[128];
    if (format_datetime_str(
            start_sec,
            start_usec,
            timezone,
            datetime_fmt,
            end_time_str,
            sizeof(end_time_str)
        ) != 0) {
        fprintf(stderr, "format_datetime_str() failed\n");
        return 1;
    }
    printf("end time:      %s\n", end_time_str);

    return 0;
}
```

#### EXAMPLE OUTPUT
```
[luke@luke utils]$ 
[luke@luke utils]$ ./build/time_util/full_example 
start time:    2026-03-05 04:40:58.705211 GMT

simulating 3 seconds of work...

Elapsed time:  00:00:03.000266
end time:      2026-03-05 04:40:58.705211 GMT
[luke@luke utils]$ 
[luke@luke utils]$ 
```

#### BUILD
```
cmake -S . -B build -DBUILD_TIME_UTIL=ON # Only build time_util
cmake --build build

cd c-common-utils/utils
cmake -S . -B build
cmake --build build
```

