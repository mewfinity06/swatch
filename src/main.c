#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

#define SKIP_FILES ".", "..", ".git"

typedef struct execution_list_s execution_list_t;
struct execution_list_s {
    const char** items;
    size_t count;
    size_t capacity;
};

typedef struct watch_file_list_s watch_file_list_t;
struct watch_file_list_s {
    const char** items;
    size_t count;
    size_t capacity;
};

struct stat_list_inner_s {
    const char* name;
    struct stat stat;
    Nob_File_Type type;
};

typedef struct stat_list_s stat_list_t;
struct stat_list_s {
    struct stat_list_inner_s* items;
    size_t count;
    size_t capacity;
};

execution_list_t execution_list   = {0};
watch_file_list_t watch_file_list = {0};
stat_list_t stat_list_buffer_1    = {0};
stat_list_t stat_list_buffer_2    = {0};

Cmd execution         = {0};
File_Paths file_paths = {0};

bool files_to_watch = false;
char cwd[256];

void usage(char*, char*, ...);
int parse_flags(int, char**);
int file_list_to_file_paths(bool);
int add_files_from_dir_recursive(const char* dir_path,
                                 File_Paths* file_paths_list);
bool is_skippable_file(char* path);
char* get_absolute_path(const char* path);
int file_paths_to_stat_list();
int update_stat_list();

int main(int argc, char** argv) {
    int err = 0;

    if ((err = parse_flags(argc, argv)) != 0) return 1;
    if ((err = file_list_to_file_paths(files_to_watch)) != 0) return 1;
    if ((err = file_paths_to_stat_list()) != 0) return 1;

    while (true)
        if ((err = update_stat_list()) != 0) return 1;
}

void usage(char* prog, char* error, ...) {
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

int parse_flags(int argc, char** argv) {
    bool in_execution_flag  = false;
    bool in_watch_file_flag = false;

    if (argc < 2) {
        usage(argv[0], "Not enough args. Expected %d or more, got %d", 2, argc);
        return 1;
    }

    for (size_t i = 0; i < (size_t)argc; ++i) {
        const char* arg = argv[i];

        // Check flags
        if (strcmp(arg, "-x") == 0) {
            in_execution_flag  = true;
            in_watch_file_flag = false;
            continue;
        }

        if (strcmp(arg, "-w") == 0) {
            files_to_watch     = true;
            in_execution_flag  = false;
            in_watch_file_flag = true;
            continue;
        }

        // Append to lists
        if (in_execution_flag) {
            nob_da_append(&execution_list, arg);
        }

        if (in_watch_file_flag) {
            nob_da_append(&watch_file_list, arg);
        }
    }

    // NOTE: This is some ugly code... but it gets the job done...
    execution = *(Cmd*)&execution_list;
    return 0;
}

int file_list_to_file_paths(bool files_to_watch) {
    if (files_to_watch) {
        for (size_t i = 0; i < watch_file_list.count; ++i) {
            const char* path_item = watch_file_list.items[i];
            char* abs_path = get_absolute_path(path_item);
            if (abs_path == NULL) {
                return 1;
            }
            Nob_File_Type type = nob_get_file_type(abs_path);

            if (type == (Nob_File_Type)-1) {
                nob_log(NOB_ERROR, "Could not get file type for %s", abs_path);
                return 1;
            }

            if (type == NOB_FILE_REGULAR) {
                nob_da_append(&file_paths, abs_path);
            } else if (type == NOB_FILE_DIRECTORY) {
                if (add_files_from_dir_recursive(abs_path, &file_paths) != 0) {
                    return 1;
                }
            }
        }
    } else {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            nob_log(NOB_INFO, "WATCHING IN: %s", cwd);
            if (add_files_from_dir_recursive(cwd, &file_paths) != 0) {
                return 1;
            }
        } else {
            nob_log(NOB_ERROR, "Could not get current working directory");
            return 1;
        }
    }
    return 0;
}

int add_files_from_dir_recursive(const char* dir_path,
                                 File_Paths* file_paths_list) {
    File_Paths children = {0};
    int result          = 0;  // 0 for success, 1 for error

    if (!nob_read_entire_dir(dir_path, &children)) {
        nob_log(NOB_ERROR, "Could not read directory %s", dir_path);
        return 1;
    }

    for (size_t i = 0; i < children.count; ++i) {
        const char* child_name = children.items[i];

        if (strcmp(child_name, ".") == 0 || strcmp(child_name, "..") == 0) {
            continue;
        }

        char* full_path    = nob_temp_sprintf("%s/%s", dir_path, child_name);
        Nob_File_Type type = nob_get_file_type(full_path);

        if (type == (Nob_File_Type)-1) {
            nob_log(NOB_ERROR, "Could not get file type for %s", full_path);
            result = 1;
            goto defer;
        }

        if (type == NOB_FILE_REGULAR) {
            nob_da_append(file_paths_list, full_path);
        } else if (type == NOB_FILE_DIRECTORY) {
            if (add_files_from_dir_recursive(full_path, file_paths_list) != 0) {
                result = 1;
                goto defer;
            }
        }
    }

defer:
    nob_da_free(children);
    return result;
}

char* get_absolute_path(const char* path) {
    char absolute_path[PATH_MAX];
    if (realpath(path, absolute_path) == NULL) {
        nob_log(NOB_ERROR, "Could not get absolute path for %s: %s", path, strerror(errno));
        return NULL;
    }
    return nob_temp_strdup(absolute_path);
}

int file_paths_to_stat_list() {  // Convert file_paths to stat_list
    for (size_t i = 0; i < file_paths.count; ++i) {
        struct stat_list_inner_s inner = {0};
        const char* file_current       = file_paths.items[i];

        nob_log(NOB_INFO, "Watching file: %s", file_current);

        if (strcmp(file_current, ".") == 0 || strcmp(file_current, "..") == 0)
            continue;

        if (!stat(file_current, &inner.stat)) {
            nob_log(NOB_ERROR, "Could not stat file '%s'", file_current);
            return 1;
        }

        inner.name = file_current;
        inner.type = nob_get_file_type(file_current);

        nob_da_append(&stat_list_buffer_1, inner);
        nob_da_append(&stat_list_buffer_2, inner);
    }
    return 0;
}

int update_stat_list() {
    assert(stat_list_buffer_1.count == stat_list_buffer_2.count &&
           "buffers should be the same size");

    for (size_t i = 0; i < stat_list_buffer_1.count; ++i) {
        struct stat_list_inner_s item_1 = stat_list_buffer_1.items[i];
        struct stat_list_inner_s item_2 = stat_list_buffer_2.items[i];

        // Update buffer 1
        if (!stat(item_1.name, &item_1.stat)) {
            nob_log(NOB_ERROR, "Could not stat file '%s'", item_1.name);
            return 1;
        };

        // Compare buffers
        if (item_1.stat.st_mtim.tv_nsec != item_2.stat.st_mtim.tv_nsec ||
            item_1.stat.st_mtim.tv_sec != item_2.stat.st_mtim.tv_sec) {
            nob_log(NOB_INFO, "File `%s` updated", item_1.name);

            if (execution.count > 0) {
                nob_log(NOB_INFO, "Executing command...");
                if (!nob_cmd_run(&execution)) {
                    nob_log(NOB_ERROR, "Command execution failed!");
                    return 1;
                }
                nob_log(NOB_INFO, "Command executed successfully.");
            }
            stat_list_buffer_2.items[i].stat = item_1.stat;
        }
    }
    sleep(1);  // Sleep for 1 second to prevent high CPU usage
    return 0;
}
