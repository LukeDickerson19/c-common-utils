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
	char buf[128];
	print(logger, fmt(buf, "formatted string: %d %c 🎉%s😄", 7, 'f', "🌟hello🚀"), .i=1);

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
-          2026-03-20 00:31:40.094914 MST  -  multiline
-          2026-03-20 00:31:40.094914 MST  -  message
-          2026-03-20 00:31:40.094914 MST  -  with
-          2026-03-20 00:31:40.094914 MST  -  prepend_datetime_fmt
-              1.9922 MiB used  -  multiline
-              1.9922 MiB used  -  message
-              1.9922 MiB used  -  with
-              1.9922 MiB used  -  prepend_memory_usage
-          2026-03-20 00:31:40.095140 MST  -      2.0547 MiB used  -  messages
 -         2026-03-20 00:31:40.095244 MST  -      2.0547 MiB used  -  |   with
 -         2026-03-20 00:31:40.095322 MST  -      2.0547 MiB used  -  |   both
  -        2026-03-20 00:31:40.095408 MST  -      2.0547 MiB used  -  |   |   and
   -       2026-03-20 00:31:40.095495 MST  -      2.0547 MiB used  -  |   |   |   indents
[luke@luke utils]$ 
[luke@luke utils]$ 
[luke@luke utils]$ 
```
