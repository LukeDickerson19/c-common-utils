# log-utils

#### DESCRIPTION

> Thread-safe logging util written in C supporting hierarchical indentation for log messages - useful for navigating logs in editors that support code folding.
> 
> Features:
> - Arbitrary indentation levels per log call (via optional int argument)
> - Handles indentation for multi-line messages
> - Microsecond datetime and memory-usage prefixes (vertically aligned without breaking indentation)
> - Overwrite the previously printed log message (via optional bool argument)
> - Output to console, log file, or both
> - Thread-safety (using single global mutex)
> 
> Traditional log levels (INFO, ERROR, etc.) are currently not yet implemented.
> This project is a rewrite of a previous [Python logging util](https://github.com/LukeDickerson19/python-common-utils/tree/master/utils/logging). Also available in [Odin lang](https://github.com/LukeDickerson19/odin-common-utils/tree/master/utils/logging_util).
> 
> Tested on:
> - Linux   (on Manjaro v25.0.10, x86_64 using gcc)
> - Windows (on Windows 11, x86_64 using clang/LLVM)

#### BUILD
```
cd c-common-utils/utils
cmake -S . -B build -DBUILD_STRING_UTIL=ON -DBUILD_TIME_UTIL=ON -DBUILD_LOGGING_UTIL=ON # build logging util and its dependency utils
cmake --build build
```

#### USAGE
Below is a quick example usage - the [tests/logging_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/logging_util/tests/logging_util_full_example.c) file shows how to use all this util's features. See [include/logging_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/logging_util/include/logging_util.h) for all function definitions and descriptions.
```c
#include "logging_util.h"

Log *logger; // global variable so you don't need to pass it to each function using it

int main(void) {

    // init any non default log settings (see include/logging_util.h for all settings)
    logger = INIT_LOG(
        .filepath = "log.txt",
        .output_to_console = true,
        .output_to_logfile = true,
        .clear_old_log = true
    );

    // log messages with different indent levels
	PRINT(logger, "a", .i=0); // no indent
	PRINT(logger, "b", .i=1); // 1 indent
	PRINT(logger, "c", .i=2); // 2 indents
	PRINT(logger, "indented\nmulti\nline\nstring", .i=3);
	PRINT(logger, FMT("formatted string: %d %c %s", 7, 'f', "hellooo"), .i=1);
	PRINT(logger, "new line before log message", .i=1, .ns=true); // ns = newline start
	PRINT(logger, "new line after log message", .i=1, .ne=true); // ne = newline end

	// prepend datetime and memory usage
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z"; // other available formats: https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
	logger->timezone = "local"; // valid options: "UTC", "local"
	PRINT(logger, "multiline\nmessage\nwith\nprepend_datetime_fmt");
    logger->prepend_datetime_fmt = NULL;
    logger->prepend_memory_usage = true;
	PRINT(logger, "multiline\nmessage\nwith\nprepend_memory_usage");
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z";
	PRINT(logger, "message", .i=0);
	PRINT(logger, "with", .i=1);
	PRINT(logger, "both", .i=1);
	PRINT(logger, "and", .i=2);
	PRINT(logger, "indents", .i=3);

    close_log(logger);
    return 0;
}

```

#### EXAMPLE OUTPUT
```
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ ./logging_util_readme_example 
a
|   b
|   |   c
|   |   |   indented
|   |   |   multi
|   |   |   line
|   |   |   string
|   formatted string: 7 f hellooo
|   
|   new line before log message
|   new line after log message
|   
-            2026-01-15 09:06:10.076000 PST  -  multiline
-            2026-01-15 09:06:10.076000 PST  -  message
-            2026-01-15 09:06:10.076000 PST  -  with
-            2026-01-15 09:06:10.076000 PST  -  prepend_datetime_fmt
-                1.6875 MiB used  -  multiline
-                1.6875 MiB used  -  message
-                1.6875 MiB used  -  with
-                1.6875 MiB used  -  prepend_memory_usage
-            2026-01-15 09:06:10.076254 PST  -      1.7266 MiB used  -  message
 -           2026-01-15 09:06:10.076343 PST  -      1.7266 MiB used  -  |   with
 -           2026-01-15 09:06:10.076422 PST  -      1.7266 MiB used  -  |   both
  -          2026-01-15 09:06:10.076498 PST  -      1.7266 MiB used  -  |   |   and
   -         2026-01-15 09:06:10.076577 PST  -      1.7266 MiB used  -  |   |   |   indents
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ 

```
