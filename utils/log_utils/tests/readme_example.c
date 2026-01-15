#include "log_utils.h"

Log *logger; // global variable so you don't need to pass it to each function using it

int main(void) {

    // init any non default log settings (see include/log_utils.h for all settings)
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
    logger->prepend_datetime_fmt = "%Y-%m-%d %H:%M:%S.%f %Z";
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
