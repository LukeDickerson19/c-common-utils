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
	print(logger, "a", .i=0); // no indent
	print(logger, "b", .i=1); // 1 indent
	print(logger, "c", .i=2); // 2 indents
	print(logger, "indented\nmulti\nline\nstring", .i=3);
	char buf[128];
	print(logger, fmt(buf, "formatted string: %d %c %s", 7, 'f', "hellooo"), .i=1);
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

    log_close(logger);
    return 0;
}
