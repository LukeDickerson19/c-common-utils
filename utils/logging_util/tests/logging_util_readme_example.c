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
	char buffer[128];
    snprintf(buffer, sizeof(buffer), "formatted string: %d %c 🎉%s😄", 7, 'f', "🌟hello🚀");
	print(logger, buffer, .i=1);

	// prepend info to each line, such as datetime and memory usage
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z"); // other available formats: https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
    set_timezone(logger, "local"); // valid options: "UTC", "local"
	print(logger, "multiline\nmessage\nwith\nprepend_datetime_fmt");
    set_prepend_datetime_fmt(logger, NULL);
    logger->prepend_memory_usage = true;
	print(logger, "multiline\nmessage\nwith\nprepend_memory_usage");
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z");
	print(logger, "messages", .i=0);
	print(logger, "with", .i=1);
	print(logger, "both", .i=1);
	print(logger, "and", .i=2);
	print(logger, "indents", .i=3);

    log_close(&logger);
    return 0;
}
