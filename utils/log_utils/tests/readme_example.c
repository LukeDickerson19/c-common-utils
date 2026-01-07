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
