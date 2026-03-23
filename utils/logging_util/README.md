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

#### BUILD
```
see [NOTES.txt](https://github.com/LukeDickerson19/c-common-utils/blob/master/NOTES.txt) section Utils, subsection Usage for the specific CLI commands to build.
Code has been tested on:
Linux (with Manjaro v25.0.10, x86_64) with both GCC and Clang
Windows (on Windows 11, x86_64 using clang/LLVM) with both MSVC (version 19.50.35723.0) and clang-cl (version 21.1.0)
```

#### USAGE
Below is a quick example usage - the [tests/logging_util_full_example.c](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/logging_util/tests/logging_util_full_example.c) file shows how to use all this util's features. See [include/logging_util.h](https://github.com/LukeDickerson19/c-common-utils/blob/master/utils/logging_util/include/logging_util.h) for all function definitions and descriptions.
```c
#include "logging_util.h"

Log *logger; // global variable so you don't need to pass it to each function using it

int main(void) {

    // init any non default log settings (see include/logging_util.h for all settings)
    logger = log_init(
        .filepath = "log.txt",
        .output_to_console = true,
        .output_to_logfile = true,
        .clear_old_log = true
    );

    // log messages with different indent levels
    print(logger, "hello", .i=0); // no indent
    print(logger, "漢字日", .i=1); // 1 indent
    print(logger, "áéöüñприв", .i=2); // 2 indents
	print(logger, "indented\nmulti\n\nline\nstring\nет你好مرحباनमस्ते←↑→↓↔↕↖↗↘↙∞\n±≈√∑©®™", .i=3);
	char fmt_buffer[128];
    snprintf(fmt_buffer, sizeof(fmt_buffer), "formatted string: %d %c 🎉%s😄", 7, 'f', "🌟hello🚀");
	print(logger, fmt_buffer, .i=1);

	// prepend info to each line, such as datetime and memory usage
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z"; // other available formats: https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
	logger->timezone = "local"; // valid options: "UTC", "local"
	print(logger, "multiline\nmessage\nwith\nprepend_datetime_fmt");
    logger->prepend_datetime_fmt = NULL;
    logger->prepend_memory_usage = true;
	print(logger, "multiline\nmessage\nwith\nprepend_memory_usage");
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z";
	print(logger, "messages", .i=0);
	print(logger, "with", .i=1);
	print(logger, "both", .i=1);
	print(logger, "and", .i=2);
	print(logger, "indents", .i=3);

    log_close(&logger);
    return 0;
}
```

#### EXAMPLE OUTPUT
```
[luke@luke utils]$ 
[luke@luke utils]$ ./build/logging_util/logging_util_readme_example 
hello
|   漢字日
|   |   áéöüñприв
|   |   |   indented
|   |   |   multi
|   |   |
|   |   |   line
|   |   |   string
|   |   |   ет你好مرحباनमस्ते←↑→↓↔↕↖↗↘↙∞
|   |   |   ±≈√∑©®™
|   formatted string: 7 f 🎉🌟hello🚀😄
-          2026-03-22 20:50:41.839192 US Mountain Standard Time  -  multiline
-          2026-03-22 20:50:41.839192 US Mountain Standard Time  -  message
-          2026-03-22 20:50:41.839192 US Mountain Standard Time  -  with
-          2026-03-22 20:50:41.839192 US Mountain Standard Time  -  prepend_datetime_fmt
-              3.2773 MiB used  -  multiline
-              3.2773 MiB used  -  message
-              3.2773 MiB used  -  with
-              3.2773 MiB used  -  prepend_memory_usage
-          2026-03-22 20:50:41.839504 US Mountain Standard Time  -      3.3203 MiB used  -  messages
 -         2026-03-22 20:50:41.839611 US Mountain Standard Time  -      3.3203 MiB used  -  |   with
 -         2026-03-22 20:50:41.839746 US Mountain Standard Time  -      3.3203 MiB used  -  |   both
  -        2026-03-22 20:50:41.839924 US Mountain Standard Time  -      3.3203 MiB used  -  |   |   and
   -       2026-03-22 20:50:41.840019 US Mountain Standard Time  -      3.3203 MiB used  -  |   |   |   indents
[luke@luke utils]$ 
[luke@luke utils]$ 
[luke@luke utils]$ 
```
