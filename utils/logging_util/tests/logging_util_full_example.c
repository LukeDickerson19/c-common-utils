#include "logging_util.h"

#include "string_util.h" // NOTE: string_util is used for its fmt*() functions and its Buffer struct. logging_util uses it under the hood but string_util isn't required to use logging_util.

#include <stdint.h> // intptr_t

///////////////// global variables ///////////////////

// directories
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
#define PATH_MAX_CHARS 1024
char BASE_DIR[PATH_MAX_CHARS];

// string util
const char *hbuf; // global heap buffer for fmt*() functions
#define HBUF_CAP 2048
#define FMT(msg, ...) fmt(hbuf, HBUF_CAP, (msg), ##__VA_ARGS__) // even more concise macro

// logging util
Log *logger; // global logging util Log struct
#define LOG(msg, ...) print(logger, (msg), ##__VA_ARGS__) // even more concise macro
#define LOGGING_ENABLED true // toggle logging entirely for ALL log structs
const char *log_filename = "log.txt";
#ifdef _WIN32
    const char *log_dir = "logging_util\\logs";
#else
    const char *log_dir = "logging_util/logs";
#endif
int ci = 0; // ci = current indentation


// thread safety test
#define THREAD_COUNT 4
#define ITERATIONS 20

//////////////////////////////////////////////////////

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <windows.h>

    // NOTE: ssize_t aleady defined in string_util.h which is imported in logging_util.h
    // typedef SSIZE_T ssize_t;

    #define open  _open
    #define read  _read
    #define write _write
    #define close _close

    static int stdout_fd(void) {
        return _fileno(stdout);
    }
    #define STDOUT_FILENO stdout_fd()

    DWORD WINAPI thread_print_loop(LPVOID arg) {
        int thread_id = (int)(intptr_t)arg;
        char *msg;
        char buf[256]; // raw char array on stack
        for (int j = 0; j < ITERATIONS; j++) {
            print(logger, fmt(buf, sizeof(buf), "thread %d iteration %d", thread_id, j), .i=1);
        }
        return 0;
    }
    int thread_safety_test() {
        HANDLE threads[THREAD_COUNT];
        for (int j = 0; j < THREAD_COUNT; j++) {
            threads[j] = CreateThread(
                NULL,
                0,
                thread_print_loop,
                (LPVOID)(intptr_t)j,
                0,
                NULL
            );
            if (!threads[j]) {
                fprintf(stderr, "CreateThread failed\n");
                return 1;
            }
        }
        WaitForMultipleObjects(THREAD_COUNT, threads, TRUE, INFINITE);
        for (int j = 0; j < THREAD_COUNT; j++)
            CloseHandle(threads[j]);
        return 0;
    }

#else
    #include <unistd.h> // used for STDOUT_FILENO and readlink

    #include <pthread.h>
    void *thread_print_loop(void *arg) {
        int thread_id = (int)(intptr_t)arg;
        char *msg;

        char buf[256]; // raw char array on stack
        for (int j = 0; j < ITERATIONS; j++) {
            print(logger, fmt(buf, sizeof(buf), "thread %d iteration %d", thread_id, j), .i=1);
        }
        return NULL;
    }
    int thread_safety_test() {

        // create and start test threads (pthread_create both creates and starts)
        pthread_t threads[THREAD_COUNT];
        for (int j = 0; j < THREAD_COUNT; j++) {
            if (pthread_create(&threads[j], NULL, thread_print_loop, (void *)(intptr_t)j) != 0) {
                perror("pthread_create");
                return 1;
            }
        }

        // block the main thread and join the test threads
        // back into main thread when they're done
        for (int j = 0; j < THREAD_COUNT; j++)
            pthread_join(threads[j], NULL);
            // pthread_join:
            // - blocks the calling thread until the thread in its first arg finishes. If the thread has already terminated before you call pthread_join(), then pthread_join() returns immediately
            // - Once the target thread has finished:
            //    - Its return value is stored in its 2nd arg *retval. NOTE: if you don't pass NULL this pointer type must match the return type of the thread's function
            // - The system reclaims the thread’s resources (stack, thread-local storage, etc.)
            // - The thread ID (pthread_t) becomes invalid / reusable
        return 0;
    }

#endif


#include <stdio.h>
#include <stdlib.h> // use for free() function
#include "cJSON.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>



void get_full_base_dir(void) {
    char exe_path[PATH_MAX_CHARS];
    char sep;
    #ifdef _WIN32
        DWORD len = GetModuleFileNameA(NULL, exe_path, PATH_MAX_CHARS);
        if (len == 0 || len == PATH_MAX_CHARS) return;
        sep = '\\';
    #else
        ssize_t len = readlink("/proc/self/exe", exe_path, PATH_MAX_CHARS - 1);
        if (len <= 0) return;
        exe_path[len] = '\0';
        sep = '/';
    #endif

    // remove executable filename from path
    char *p = strrchr(exe_path, sep); // strrchr() finds the last occurrence of a specific character within a string
    if (!p) return;
    *p = '\0';

    // remove 1 parent directory/ies
    int num_lvls; // number of directory levels to go up
    #if defined(_WIN32) && defined(_MSC_VER)
        // MSVC compiler (only available on windows) puts the executable in an added Release (or Debug) directory
        num_lvls = 3;
    #else
        num_lvls = 2;
    #endif
    for (int i = 0; i < num_lvls; i++) {
        // remove last directory from path
        p = strrchr(exe_path, sep);
        if (!p) return;
        *p = '\0';
    }

    strncpy(BASE_DIR, exe_path, PATH_MAX_CHARS);
    printf("BASE_DIR:     %s\n", BASE_DIR); fflush(stdout); // print immediately (no buffer)
}

void test_print() {
    ci = 0;
    print(logger, "\ntest_print():", .i=ci);
    ci += 1;

    // test num_indents and multi line indentation
    print(logger, "abc123", .i=ci);
    print(logger, "漢字日", .i=ci+1);
    LOG("áéöüñприв", .i=ci+2);
    LOG("ет你好مرحباनमस्ते←↑→↓↔↕↖↗↘↙∞", .i=ci+3);
    LOG("±≈√∑©®™🌟🚀😄🐍🏖️🎉", .i=ci+4);
    LOG("\nindented\nmulti\nline\nstring", .i=ci+5);

    // test formatted string
    char buf[256];
    print(logger, fmt(buf, sizeof(buf), "formatted string: %d %c %s 😄%s🐍", 7, 'f', "hellooo", "🏖️🎉"), .i=ci);
    LOG(FMT("hello %s", "world"), .i=ci);

    // test new line start
    LOG("new line start = true, draw line = false", .i=ci, .ns=true);
    LOG("new line start = true, draw line = true", .i=ci, .ns=true, .d=true);
    LOG("new line start = false", .i=ci, .ns=false);

    // test new line end
    LOG("new line end = true, draw line = false", .i=ci, .ne=true);
    LOG("new line end = True, draw line = True", .i=ci, .ne=true, .d=true);
    LOG("new line end = false", .i=ci, .ne=false);

    // test get latest msg
    char *console_msg, *logfile_msg;
    LOG("latest log message", .i=ci+1, .ns=true, .ne=true);
    console_msg = get_latest_console_msg(logger);
    logfile_msg = get_latest_logfile_msg(logger);
    fwrite(console_msg, 1, strlen(console_msg), stdout); // if theres indents, it was preserved
    fwrite(logfile_msg, 1, strlen(logfile_msg), stdout);
    LOG("latest\nmulti-line\n\nlog\nmessage", .i=ci+2, .ns=true, .ne=true);
    console_msg = get_latest_console_msg(logger);
    logfile_msg = get_latest_logfile_msg(logger);
    fwrite(console_msg, 1, strlen(console_msg), stdout); // if theres indents, it was preserved
    fwrite(logfile_msg, 1, strlen(logfile_msg), stdout);

    // test prepend only datetime
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z");
    set_timezone(logger, "local");
    logger->prepend_elapsed_time = false;
    logger->prepend_memory_usage = false;
    LOG("testing single line prepend_datetime_fmt w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_datetime_fmt", .i=ci);
    LOG("testing single line indented prepend_datetime_fmt", .i=ci+1);

    // test prepend only elapsed time
    set_prepend_datetime_fmt(logger, NULL);
    logger->prepend_elapsed_time = true;
    logger->prepend_memory_usage = false;
    LOG("testing single line prepend_elapsed_time w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_elapsed_time", .i=ci);
    LOG("testing single line indented prepend_elapsed_time", .i=ci+1);

    // test prepend only memory usage
    set_prepend_datetime_fmt(logger, NULL);
    logger->prepend_elapsed_time = false;
    logger->prepend_memory_usage = true;
    LOG("testing single line prepend_memory_usage w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_memory_usage", .i=ci);
    LOG("testing single line indented prepend_memory_usage", .i=ci+1);

    // test both prepend datetime and prepend elapsed time
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z");
    logger->prepend_elapsed_time = true;
    logger->prepend_memory_usage = false;
    LOG("testing single line prepend_datetime_fmt and prepend_elapsed_time w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_datetime_fmt\nand\nprepend_elapsed_time", .i=ci);
    LOG("testing single line indented prepend_datetime_fmt and prepend_elapsed_time", .i=ci+1);

    // test both prepend datetime and prepend memory usage
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z");
    logger->prepend_elapsed_time = false;
    logger->prepend_memory_usage = true;
    LOG("testing single line prepend_datetime_fmt and prepend_memory_usage w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_datetime_fmt\nand\nprepend_memory_usage", .i=ci);
    LOG("testing single line indented prepend_datetime_fmt and prepend_memory_usage", .i=ci+1);

    // test both prepend elapsed time and prepend memory usage
    set_prepend_datetime_fmt(logger, NULL);
    logger->prepend_elapsed_time = true;
    logger->prepend_memory_usage = true;
    LOG("testing single line prepend_elapsed_time and prepend_memory_usage w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend_elapsed_time\nand\nprepend_memory_usage", .i=ci);
    LOG("testing single line indented prepend_elapsed_time and prepend_memory_usage", .i=ci+1);

    // test all 3 prependable information
    set_prepend_datetime_fmt(logger, "%Y-%m-%d %H:%M:%S.%f %Z");
    logger->prepend_elapsed_time = true;
    logger->prepend_memory_usage = true;
    LOG("testing single line prepend all 3 w/out indent", .ns=true);
    LOG("testing\nmulti\nline\n\nprepend\nall\n3", .i=ci);
    LOG("testing single line indented prepend all 3", .i=ci+1);
    set_prepend_datetime_fmt(logger, NULL);
    logger->prepend_elapsed_time = false;
    logger->prepend_memory_usage = false;

    // test line and message truncation
    LOG("truncation tests:", .i=ci, .ns=true);
    int default_max_message_len = logger->max_message_len;
    int default_max_line_len = logger->max_line_len;
    int test_max_message_len = 500;
    int test_max_line_len = 100;
    // char *long_line = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ "; // ASCII characters
    char *long_line = "ABCDEFGHIJKLMNOPQRSTUVWXYZ12345!@#$%^&*()-+_=漢字日本水áéöüñпривет你好مرحباनमस्ते←↑→↓↔↕↖↗↘↙∞±≈√∑©®™🌟🚀😄🐍🏖️🎉"; // example UTF-8 characters

    // test message truncation
    logger->max_message_len = test_max_message_len;
    LOG(FMT("Test message truncation: set logger->max_message_len to %zd", logger->max_message_len), .i=ci+1, .ns=true);
    LOG(FMT("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s", long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line), .i=ci+2);

    // test line truncation
    logger->max_line_len = test_max_line_len;
    LOG(FMT("Test line truncation: set logger->max_line_len to %zd", logger->max_line_len), .i=ci+1, .ns=true);
    LOG(long_line, .i=ci+2);

    // test both
    LOG("Test both:", .i=ci+1, .ns=true);
    LOG(FMT("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s", long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line, long_line), .i=ci+2);

    // test complete, restore default settings
    logger->max_message_len = default_max_message_len;
    logger->max_line_len    = default_max_line_len;
    LOG("truncation tests complete.", .i=ci, .ns=true, .d=true);
    LOG(FMT("restored logger->max_line_len    to default: %zd", logger->max_line_len), .i=ci);
    LOG(FMT("restored logger->max_message_len to default: %zd", logger->max_message_len), .i=ci, .ne=true);

}

cJSON *create_example_json(void) {
	/* create this json:

		{
			"name": "Awesome 4K",
			"resolutions": [
				{
					"width": 1280,
					"height": 720
				},
				{
					"width": 1920,
					"height": 1080
				},
				{
					"width": 3840,
					"height": 2160
				}
			]
		}
	
	NOTE: you are required to free it after use
	*/

	const unsigned int resolution_numbers[3][2] = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };
    char *string = NULL;
    cJSON *name = NULL;
    cJSON *resolutions = NULL;
    cJSON *resolution = NULL;
    cJSON *width = NULL;
    cJSON *height = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL) goto end;

    name = cJSON_CreateString("Awesome 4K");
    if (name == NULL) goto end;

    /* after creation was successful, immediately add it to the monitor,
     * thereby transferring ownership of the pointer to it */
    cJSON_AddItemToObject(monitor, "name", name);

    resolutions = cJSON_CreateArray();
    if (resolutions == NULL) goto end;

    cJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index) {
        resolution = cJSON_CreateObject();
        if (resolution == NULL) goto end;
        cJSON_AddItemToArray(resolutions, resolution);

        width = cJSON_CreateNumber(resolution_numbers[index][0]);
        if (width == NULL) goto end;
        cJSON_AddItemToObject(resolution, "width", width);

        height = cJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL) goto end;
        cJSON_AddItemToObject(resolution, "height", height);
    }

	return monitor;
	end:
		cJSON_Delete(monitor);
		return NULL;
}
static char *read_all_file_content(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) return NULL;

    size_t cap = (st.st_size > 0) ? (size_t)st.st_size : 4096;
    char *buf = malloc(cap + 1);
    if (!buf) return NULL;

    size_t len = 0;
    while (1) {
        ssize_t r = read(fd, buf + len, cap - len);
        if (r < 0) {
            free(buf);
            return NULL;
        }
        if (r == 0)
            break;
        len += r;
        if (len == cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap + 1);
            if (!tmp) {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
    }
    buf[len] = '\0';
    return buf;
}
cJSON *read_json_file(const char *filepath) {
    if (!filepath) return NULL;
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return NULL;
    char *text = read_all_file_content(fd);
    close(fd);
    if (!text) return NULL;
    cJSON *json = cJSON_Parse(text);
    free(text);
    return json;  // caller owns it
}
int write_json_file(cJSON *json, const char *filepath) {
    if (!json || !filepath) return -1;
    char *out = cJSON_Print(json); // with newlines and indents
    if (!out) return -1;
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        free(out);
        return -1;
    }
    size_t len = strlen(out);
    size_t written = 0;
    while (written < len) {
        ssize_t rc = write(fd, out + written, len - written);
        if (rc <= 0) {
            close(fd);
            free(out);
            return -1;
        }
        written += (size_t)rc;
    }
    #if defined(_WIN32)
        _commit(fd);   // flush file buffers
    #else
        fsync(fd);
    #endif
    close(fd);
    free(out);
    return 0;
}

void test_print_json() {
    LOG("\ntest_print_json():");

    // json object struct members:
    // https://github.com/DaveGamble/cJSON/tree/v1.7.19?tab=readme-ov-file#data-structure

    /* cJSON library:

		Printing JSON
			source: https://github.com/DaveGamble/cJSON/tree/v1.7.19?tab=readme-ov-file#printing-json

			"Given a tree of cJSON items, you can print them as a string using cJSON_Print.

			char *string = cJSON_Print(json);

			It will allocate a string and print a JSON representation of the tree into it. Once it returns, you are fully responsible for deallocating it after use with your allocator. (usually free, depends on what has been set with cJSON_InitHooks).

			cJSON_Print will print with whitespace for formatting. If you want to print without formatting, use cJSON_PrintUnformatted.

			If you have a rough idea of how big your resulting string will be, you can use cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt). fmt is a boolean to turn formatting with whitespace on and off. prebuffer specifies the first buffer size to use for printing. cJSON_Print currently uses 256 bytes for its first buffer size. Once printing runs out of space, a new buffer is allocated and the old gets copied over before printing is continued.

			These dynamic buffer allocations can be completely avoided by using cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format). It takes a buffer to a pointer to print to and its length. If the length is reached, printing will fail and it returns 0. In case of success, 1 is returned. Note that you should provide 5 bytes more than is actually needed, because cJSON is not 100% accurate in estimating if the provided memory is enough."
	
	*/

    LOG("\nExample 1:", .i=1);
    cJSON *example_json = create_example_json();
    // char *json_string = cJSON_print(example_json);
    char json_string[1024]; // 1 KB buffer
    if (cJSON_PrintPreallocated(example_json, json_string, sizeof(json_string), cJSON_False) != 1) {
    	fprintf(stderr, "Failed to print json_string.\n");
    } else {
    	LOG(json_string, .i=2);
    }
    cJSON_Delete(example_json);
    // free(json_string);

    // json file read/write example
    LOG("\nExample 2:", .i=1);
    char *filename = "json_example.json";
    char filepath[PATH_MAX_CHARS];
    #ifdef _WIN32
        fmt(filepath, PATH_MAX_CHARS, "%s\\logging_util\\test_output\\%s", BASE_DIR, filename);
    #else
        fmt(filepath, PATH_MAX_CHARS, "%s/logging_util/test_output/%s", BASE_DIR, filename);
    #endif
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "Luke");
    cJSON_AddNumberToObject(root, "age", 30);
    cJSON *langs = cJSON_AddArrayToObject(root, "languages");
    cJSON_AddItemToArray(langs, cJSON_CreateString("C"));
    cJSON_AddItemToArray(langs, cJSON_CreateString("Python"));
    int rc = write_json_file(root, filepath);
    if (rc != 0) {
        LOG(FMT("Failed to write JSON, error code %d", rc), .i=2);
        LOG(FMT("json filepath: %s", filepath), .i=3);
    }
    cJSON_Delete(root);
    cJSON *loaded = read_json_file(filepath);
    if (!loaded) {
        LOG("Failed to read JSON", .i=2);
        LOG(FMT("json filepath: %s", filepath), .i=3);
        return;
    }
    char *loaded_string = cJSON_Print(loaded);
    cJSON *name = cJSON_GetObjectItem(loaded, "name");
    cJSON *age  = cJSON_GetObjectItem(loaded, "age");
    LOG(loaded_string, .i=2);
    if (cJSON_IsString(name))
        LOG(FMT("name = %s", name->valuestring), .i=2);
    if (cJSON_IsNumber(age))
        LOG(FMT("age = %d", age->valueint), .i=2);
        cJSON_Delete(loaded);
    free(loaded_string);

}

void test_overwrite_prev_msg() {
	LOG("\ntest_overwrite_prev_msg():");

    int sleep_time = 500000; // in microseconds
    int i = 1;

    // new text has shorter lines
	LOG("aaaa", .i=i, .overwrite_prev_msg=false);
	sleep_microseconds(sleep_time);
	LOG("bbb", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("cc", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("d", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("", .i=0, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);

    // new text has longer lines
	LOG("a", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("bb", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("ccc", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("dddd", .i=i, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);
	LOG("", .i=0, .overwrite_prev_msg=true);
	sleep_microseconds(sleep_time);

    // new text has more lines
    LOG("a", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("b\nb", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("c\nc\nc", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("d\nd\nd\nd", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("", .i=0, .end="", .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);

    // new text has less lines
    LOG("a\na\na\na", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("b\nb\nb", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("c\nc", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("d", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("", .i=0, .end="", .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);

	// verify regular log.print() works after overwrite_prev_msg
    LOG("a", .i=0);
    LOG("b", .i=1);
    LOG("c", .i=2);
    LOG("d", .i=3);
    LOG("e", .i=4);
    LOG("indented\nmulti\nline\nstring", .i=5);

    // verify overwrite_prev_msg works after regular log.print()
    LOG("test", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("overwrite_prev_msg", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("after", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("regular print()", .i=i, .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);
    LOG("", .i=0, .end="", .overwrite_prev_msg=true);
    sleep_microseconds(sleep_time);

    LOG("test regular print() after overwrite_prev_msg", .i=i, .ne=true);

	LOG(FMT("log file with final test_overwrite_prev_msg output at:\n%s", logger->filepath), .i=i);
	LOG(FMT("console indent  = \"%s\"", logger->console_indent), .i=i+1);
	LOG(FMT("log file indent = \"%s\"", logger->logfile_indent), .i=i+1, .ne=true);

}

void test_thread_safety() {
    LOG("\ntest_thread_safety():");
    logger->thread_safe = true;
    if (thread_safety_test() != 0) {
        fprintf(stderr, "Thread safety test failed\n");
    } else {
        LOG(FMT("test passes if all %d x %d thread/iteration combinations were printed (order does\'t matter)", THREAD_COUNT, ITERATIONS), .ns=true);
        LOG(FMT("test complete, log file at:\n%s", logger->filepath), .ne=true);
    }
}

int main(void) {

    // init global heap buffer for string_util fmt*() functions
    hbuf = malloc(HBUF_CAP);

    // Set log file path
    get_full_base_dir();
    char log_filepath[256]; fmt(log_filepath, sizeof(log_filepath),
        "%s%c%s%c%s", BASE_DIR, sep, log_dir, sep, log_filename);
    printf("log filepath: %s\n", log_filepath); fflush(stdout); // print immediately (no buffer)

    // initialize log struct
    logger = log_init(
        .enabled = LOGGING_ENABLED,
        .filepath = log_filepath,
        .output_to_console = true,
        .output_to_logfile = true,
        .clear_old_log = true
    ); // closest i could get to setting optional args for a c struct initilization, could not pass struct pointer and return error code. if logger is NULL, then init failed
    if (!logger) {
        fprintf(stderr, "Failed to initialize logger.\n");
        return -1;
    }

    // run test functions
    test_print();
	test_print_json();
	test_overwrite_prev_msg();
    test_thread_safety();

    // free logger and heap buffer
    log_close(&logger);
    free(hbuf);

    return 0;
}


