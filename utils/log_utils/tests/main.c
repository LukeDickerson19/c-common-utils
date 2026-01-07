#include "log_utils.h"

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <windows.h>

    typedef long ssize_t;

    #define open  _open
    #define read  _read
    #define write _write
    #define close _close

    static void sleep(unsigned int seconds) {
        Sleep(seconds * 1000);
    }

    static int stdout_fd(void) {
        return _fileno(stdout);
    }
    #define STDOUT_FILENO stdout_fd()

#else
    #include <unistd.h> // used for STDOUT_FILENO and readlink
#endif


#include <stdio.h>
#include <stdlib.h> // use for free() function
#include "cJSON.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


#define PATH_MAX_CHARS 1024
char BASE_DIR[PATH_MAX_CHARS];


Log lg = DEFAULT_LOG;


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

    // remove last directory from path
    p = strrchr(exe_path, sep);
    if (!p) return;
    *p = '\0';

    strncpy(BASE_DIR, exe_path, PATH_MAX_CHARS);
    printf("BASE_DIR:     %s\n", BASE_DIR); fflush(stdout); // print immediately (no buffer)
}

void test_print() {
	PRINT(&lg, "\ntest_print():");

	// test num_indents and multi line indentation
	PRINT(&lg, "a", .i=0);
	PRINT(&lg, "b", .i=1);
	PRINT(&lg, "c", .i=2);
	PRINT(&lg, "d", .i=3);
	PRINT(&lg, "e", .i=4);
	PRINT(&lg, "indented\nmulti\nline\nstring", .i=5);
	PRINT(&lg, FMT("formatted string: %d %c %s", 7, 'f', "hellooo"), .i=1);

	// test new line start
	PRINT(&lg, "new line start = true, draw line = false", .i=1, .ns=true);
	PRINT(&lg, "new line start = true, draw line = true", .i=1, .ns=true, .d=true);
	PRINT(&lg, "new line start = false", .i=1, .ns=false);

	// test new line end
	PRINT(&lg, "new line end = true, draw line = false", .i=1, .ne=true);
	PRINT(&lg, "new line end = True, draw line = True", .i=1, .ne=true, .d=true);
	PRINT(&lg, "new line end = false", .i=1, .ne=false);

	// test prepend datetime
    lg.prepend_datetime_fmt = "%y-%m-%d %H:%M:%S.%f %Z";
    lg.timezone = "local"; // valid options: "UTC", "local"
	PRINT(&lg, "testing single line prepend_datetime_fmt", .ns=true);
	PRINT(&lg, "testing\nmulti\nline\nprepend_datetime_fmt");
	PRINT(&lg, "testing single line indented prepend_datetime_fmt", .i=1);

	// test prepend memory usage
    lg.prepend_datetime_fmt = NULL;
    lg.prepend_memory_usage = true;
	PRINT(&lg, "testing single line prepend_memory_usage", .ns=true);
	PRINT(&lg, "testing\nmulti\nline\nprepend_memory_usage");
	PRINT(&lg, "testing single line indented prepend_memory_usage", .i=1);

	// test both prepend datetime and memory usage
    lg.prepend_datetime_fmt = "%y-%m-%d %H:%M:%S.%f %Z";
    lg.prepend_memory_usage = true;
	PRINT(&lg, "testing single line prepend_datetime_fmt and prepend_memory_usage", .ns=true);
	PRINT(&lg, "testing\nmulti\nline\nprepend_datetime_fmt\nand\nprepend_memory_usage");
	PRINT(&lg, "testing single line indented prepend_datetime_fmt and prepend_memory_usage", .i=1);
	lg.prepend_datetime_fmt = NULL;
    lg.prepend_memory_usage = false;

    // test line and message truncation
    // make sure MAX_LINE_CHARS and MAX_MESSAGE_CHARS in log_utils.h are small values for testing
    PRINT(&lg, "Test line truncation: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50", .i=1, .ns=true);
    PRINT(&lg, "Test message truncation:\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50\n1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50\n", .i=1, .ns=true);
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
	PRINT(&lg, "\ntest_print_json():");

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

	PRINT(&lg, "\nExample 1:", .i=1);
	cJSON *example_json = create_example_json();
	// char *json_string = cJSON_Print(example_json);
	char json_string[1024]; // 1 KB buffer
	if (cJSON_PrintPreallocated(example_json, json_string, sizeof(json_string), cJSON_False) != 1) {
		fprintf(stderr, "Failed to print json_string.\n");
	} else {
		PRINT(&lg, json_string, .i=2);
	}
	cJSON_Delete(example_json);
	// free(json_string);

    // json file read/write example
	PRINT(&lg, "\nExample 2:", .i=1);
	char *filename = "json_example.json";
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "Luke");
    cJSON_AddNumberToObject(root, "age", 30);
    cJSON *langs = cJSON_AddArrayToObject(root, "languages");
    cJSON_AddItemToArray(langs, cJSON_CreateString("C"));
    cJSON_AddItemToArray(langs, cJSON_CreateString("Python"));
	if (write_json_file(root, filename) != 0) {
        PRINT(&lg, "Failed to write JSON\n");
    }
    cJSON_Delete(root);
    cJSON *loaded = read_json_file(filename);
    if (!loaded) {
        PRINT(&lg, "Failed to read JSON\n");
        return;
    }
	char *loaded_string = cJSON_Print(loaded);
    cJSON *name = cJSON_GetObjectItem(loaded, "name");
    cJSON *age  = cJSON_GetObjectItem(loaded, "age");
	PRINT(&lg, loaded_string, .i=2);
    if (cJSON_IsString(name))
        PRINT(&lg, FMT("name = %s", name->valuestring), .i=2);
    if (cJSON_IsNumber(age))
        PRINT(&lg, FMT("age = %d", age->valueint), .i=2);
    cJSON_Delete(loaded);
	free(loaded_string);

}

void test_overwrite_prev_print() {
	PRINT(&lg, "\ntest_overwrite_prev_print():");

    int sleep_time = 1; // seconds
    int i = 1;

    // new text has shorter lines
	PRINT(&lg, "aaaa", .i=i, .overwrite_prev_print=false);
	sleep(sleep_time);
	PRINT(&lg, "bbb", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "cc", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "d", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "", .i=0, .overwrite_prev_print=true);
	sleep(sleep_time);

    // new text has longer lines
	PRINT(&lg, "a", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "bb", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "ccc", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "dddd", .i=i, .overwrite_prev_print=true);
	sleep(sleep_time);
	PRINT(&lg, "", .i=0, .overwrite_prev_print=true);
	sleep(sleep_time);

    // new text has more lines
    PRINT(&lg, "a", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "b\nb", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "c\nc\nc", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "d\nd\nd\nd", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "", .i=0, .end="", .overwrite_prev_print=true);
    sleep(sleep_time);

    // new text has less lines
    PRINT(&lg, "a\na\na\na", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "b\nb\nb", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "c\nc", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "d", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "", .i=0, .end="", .overwrite_prev_print=true);
    sleep(sleep_time);

	// verify regular log.print() works after overwrite_prev_print
    PRINT(&lg, "a", .i=0);
    PRINT(&lg, "b", .i=1);
    PRINT(&lg, "c", .i=2);
    PRINT(&lg, "d", .i=3);
    PRINT(&lg, "e", .i=4);
    PRINT(&lg, "indented\nmulti\nline\nstring", .i=5);

    // verify overwrite_prev_print works after regular log.print()
    PRINT(&lg, "test", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "overwrite_prev_print", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "after", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "regular print()", .i=i, .overwrite_prev_print=true);
    sleep(sleep_time);
    PRINT(&lg, "", .i=0, .end="", .overwrite_prev_print=true);
    sleep(sleep_time);

    PRINT(&lg, "test regular print() after overwrite_prev_print", .i=0, .ne=true);

}



int main(void) {
    get_full_base_dir();
    #ifdef _WIN32
        lg.filepath = FMT("%s\\log\\log.txt", BASE_DIR);
    #else
        lg.filepath = FMT("%s/log/log.txt", BASE_DIR);
    #endif
    printf("log filepath: %s\n", lg.filepath); fflush(stdout); // print immediately (no buffer)

    // closest i could get to setting optional args for a c struct initilization:
    lg.output_to_console = true;
    lg.output_to_logfile = true;
    lg.clear_old_log = true;
    init_log(&lg);

    // test_print();
	// test_print_json();
	test_overwrite_prev_print();

    close_log(&lg);
    return 0;
}
