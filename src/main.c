#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

typedef struct execution_list_s execution_list_t;
struct execution_list_s {
    const char **items;
    size_t count;
    size_t capacity;
};

typedef struct watch_file_list_s watch_file_list_t;
struct watch_file_list_s {
    const char **items;
    size_t count;
    size_t capacity;
};

typedef struct stat_list_s stat_list_t;
struct stat_list_s {
    struct stat *items;
    size_t count;
    size_t capacity;
};

execution_list_t execution_list = {0};
watch_file_list_t watch_file_list = {0};
stat_list_t stat_list_buffer_1 = {0};
stat_list_t stat_list_buffer_2 = {0};

Cmd execution = {0};
File_Paths file_paths = {0};

bool files_to_watch = false;
char cwd[256];

void usage(char *, char *, ...);
int parse_flags(int, char **);
int file_list_to_file_paths(bool);
int file_paths_to_stat_list();
int update_stat_list();

int main(int argc, char **argv) {
    int err = 0;

    if ((err = parse_flags(argc, argv)) != 0)
        return 1;

    if ((err = file_list_to_file_paths(files_to_watch)) != 0)
        return 1;

    if ((err = file_paths_to_stat_list()) != 0)
        return 1;
    while (true)
        if ((err = update_stat_list()) != 0)
            return 1;
}

void usage(char *prog, char *error, ...) {
    va_list args;
    va_start(args, error);

    char error_real[256];

    nob_log(NOB_INFO, "USAGE: %s [args]", prog);
    if (error != NULL) {
        vsprintf(error_real, error, args);
        nob_log(NOB_ERROR, "Error: %s", error_real);
    }

    va_end(args);
}

int parse_flags(int argc, char **argv) {
    bool in_execution_flag = false;
    bool in_watch_file_flag = false;

    if (argc < 2) {
        usage(argv[0], "Not enough args. Expected %d or more, got %d", 2, argc);
        return 1;
    }

    for (size_t i = 0; i < (size_t)argc; ++i) {
        const char *arg = argv[i];

        // Check flags
        if (strcmp(arg, "-x") == 0) {
            in_execution_flag = true;
            in_watch_file_flag = false;
            continue;
        }

        if (strcmp(arg, "-w") == 0) {
            files_to_watch = true;
            in_execution_flag = false;
            in_watch_file_flag = true;
            continue;
        }

        // Append to lists
        if (in_execution_flag) {
            da_append(&execution_list, arg);
        }

        if (in_watch_file_flag) {
            da_append(&watch_file_list, arg);
        }
    }

    // NOTE: This is some ugly code... but it gets the job done...
    execution = *(Cmd *)&execution_list;
    return 0;
}

int file_list_to_file_paths(bool files_to_watch) {
    // Add files to file_paths
    if (files_to_watch) {
        // Check each file
        nob_log(NOB_ERROR, "%s:%d Unimeplemented", __FILE__, __LINE__);
        return 1;
    } else {
        // Get each file in current working directory
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            nob_log(NOB_INFO, "WATCHING IN: %s", cwd);
            if (!read_entire_dir(cwd, &file_paths))
                return 1;
        } else {
            nob_log(NOB_ERROR, "Could not get current working directory");
            return 1;
        }
    }
    return 0;
}

int file_paths_to_stat_list() { // Convert file_paths to stat_list
    for (size_t i = 0; i < file_paths.count; ++i) {
        struct stat current;
        const char *file_current = file_paths.items[i];

        if (strcmp(file_current, ".") || strcmp(file_current, ".."))
            continue;

        if (!stat(file_current, &current)) {
            nob_log(NOB_ERROR, "Could not stat file '%s'", file_current);
            return 1;
        }

        da_append(&stat_list_buffer_1, current);
        da_append(&stat_list_buffer_2, current);
    }
    return 0;
}

int update_stat_list() {
    assert(stat_list_buffer_1.count == stat_list_buffer_2.count &&
           "buffers should be the same size");
    return 0;
}
