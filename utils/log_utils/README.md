# log-utils

#### DESCRIPTION

> Logging util written in C that supports hierarchical indentation for log messages — useful for visually organizing program flow and navigating logs in editors that support code folding.
> 
> Features:
> - Arbitrary indentation levels per log call
> - Correct handling of multi-line string indentation
> - Optional datetime and memory-usage prefixes that don't break indentation
> - Output to console, file, or both
> - Optional overwrite of the previously printed log message
> 
> Traditional log levels (INFO, ERROR, etc.) are currently not yet implemented.
> This project is a rewrite of a previous [Python logging util](https://github.com/LukeDickerson19/python-common-utils/tree/master/utils/logging).
> 
> Tested on:
> - Linux   (on Manjaro v25.0.10, x86_64 using gcc)
> - Windows (on Windows 11, x86_64 using clang/LLVM)

#### USAGE
Below is an example usage. The tests/main.c file shows more thoroughly how to use this util's features.
```
#include "log_utils.h"

Log lg = DEFAULT_LOG; // global variable so you don't need to pass it to each function

int main(void) {

    // init any non default settings (see include/log_utils.h for all settings)
    lg.filepath = "log.txt";
    lg.output_to_console = true;
    lg.output_to_logfile = true;
    lg.clear_old_log = true;
    init_log(&lg);

    // log stuff
	PRINT(&lg, "a", .i=0); // no indent
	PRINT(&lg, "b", .i=1); // 1 indent
	PRINT(&lg, "c", .i=2); // 2 indents
	PRINT(&lg, "indented\nmulti\nline\nstring", .i=3);
	PRINT(&lg, FMT("formatted string: %d %c %s", 7, 'f', "hellooo"), .i=1);
	PRINT(&lg, "new line before log message", .i=1, .ns=true); // ns = newline start
	PRINT(&lg, "new line after log message", .i=1, .ne=true); // ne = newline end

	// prepend datetime and memory usage
    lg.prepend_datetime_fmt = "%y-%m-%d %H:%M:%S.%f %Z";
	PRINT(&lg, "multiline\nmessage\nwith\nprepend_datetime_fmt");
    lg.prepend_datetime_fmt = NULL;
    lg.prepend_memory_usage = true;
	PRINT(&lg, "multiline\nmessage\nwith\nprepend_memory_usage");
    lg.prepend_datetime_fmt = "%y-%m-%d %H:%M:%S.%f %Z";
	PRINT(&lg, "message", .i=0);
	PRINT(&lg, "with", .i=1);
	PRINT(&lg, "both", .i=1);
	PRINT(&lg, "and", .i=2);
	PRINT(&lg, "indents", .i=3);

    close_log(&lg);
    return 0;
}

```

#### EXAMPLE OUTPUT
```
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ ./readme_example 
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
-            26-01-06 17:00:40.277899 GMT  -  multiline
-            26-01-06 17:00:40.277899 GMT  -  message
-            26-01-06 17:00:40.277899 GMT  -  with
-            26-01-06 17:00:40.277899 GMT  -  prepend_datetime_fmt
-            1.6719 MiB used  -  multiline
-            1.6719 MiB used  -  message
-            1.6719 MiB used  -  with
-            1.6719 MiB used  -  prepend_memory_usage
-            26-01-06 17:00:40.278052 GMT  -  1.7148 MiB used  -  message
 -           26-01-06 17:00:40.278098 GMT  -  1.7148 MiB used  -  |   with
 -           26-01-06 17:00:40.278140 GMT  -  1.7148 MiB used  -  |   both
  -          26-01-06 17:00:40.278191 GMT  -  1.7148 MiB used  -  |   |   and
   -         26-01-06 17:00:40.278235 GMT  -  1.7148 MiB used  -  |   |   |   indents
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ 
[luke@luke build]$ 

```
