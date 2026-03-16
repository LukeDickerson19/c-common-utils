# log-utils

> [!NOTE]
> This is an older version of the logging_util code before it was made to use string_util and time_util.

#### DESCRIPTION
> Thread-safe logging util written in C supporting hierarchical indentation for log messages — useful for navigating logs in editors that support code folding.
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

#### USAGE
Below is a quick example usage - the tests/logging_util_full_example.c file shows how to use all this util's features.
```c
#include "logging_util.h"

Log *logger; // global variable so you don't need to pass it to each function using it

int main(void) {

    // init any non default log settings (see include/logging_util.h for all settings)
    logger = init_log(
        .filepath = "log.txt",
        .output_to_console = true,
        .output_to_logfile = true,
        .clear_old_log = true
    );

    // log messages with different indent levels
	print(logger, "a", .i=0); // no indent
	print(logger, "b", .i=1); // 1 indent
	print(logger, "c", .i=2); // 2 indents
	print(logger, "indented\nmulti\nline\nstring", .i=3);
	char buffer[128];
	print(logger, fmt(buffer, "formatted string: %d %c %s", 7, 'f', "hellooo"), .i=1);
	print(logger, "new line before log message", .i=1, .ns=true); // ns = newline start
	print(logger, "new line after log message", .i=1, .ne=true); // ne = newline end

	// prepend info to each line, such as datetime and memory usage
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z"; // other available formats: https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
	logger->timezone = "local"; // valid options: "UTC", "local"
	print(logger, "multiline\nmessage\nwith\nprepend_datetime_fmt");
    logger->prepend_datetime_fmt = NULL;
    logger->prepend_memory_usage = true;
	print(logger, "multiline\nmessage\nwith\nprepend_memory_usage");
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z";
	print(logger, "message", .i=0);
	print(logger, "with", .i=1);
	print(logger, "both", .i=1);
	print(logger, "and", .i=2);
	print(logger, "indents", .i=3);

    close_log(logger);
    return 0;
}
```

#### EXAMPLE OUTPUT
```
[luke@luke original_stand_alone_logging_util]$ 
[luke@luke original_stand_alone_logging_util]$ 
[luke@luke original_stand_alone_logging_util]$ ./build/logging_util_readme_example 
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
-          2026-03-15 19:37:19.765003 MST  -  multiline
-          2026-03-15 19:37:19.765003 MST  -  message
-          2026-03-15 19:37:19.765003 MST  -  with
-          2026-03-15 19:37:19.765003 MST  -  prepend_datetime_fmt
-              1.7891 MiB used  -  multiline
-              1.7891 MiB used  -  message
-              1.7891 MiB used  -  with
-              1.7891 MiB used  -  prepend_memory_usage
-          2026-03-15 19:37:19.765276 MST  -      1.8516 MiB used  -  message
 -         2026-03-15 19:37:19.765341 MST  -      1.8516 MiB used  -  |   with
 -         2026-03-15 19:37:19.765392 MST  -      1.8516 MiB used  -  |   both
  -        2026-03-15 19:37:19.765442 MST  -      1.8516 MiB used  -  |   |   and
   -       2026-03-15 19:37:19.765491 MST  -      1.8516 MiB used  -  |   |   |   indents
[luke@luke original_stand_alone_logging_util]$ 
[luke@luke original_stand_alone_logging_util]$ 
[luke@luke original_stand_alone_logging_util]$ 
```
