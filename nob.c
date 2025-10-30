#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define MAIN_EXE_NAME "swatch"
#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"

#define FLAGS "-Wall", "-Wextra", "-pedantic", "-Werror"

File_Paths src_paths = {0};
Cmd build_cmd = {0};
Cmd run_cmd = {0};

void usage(char *prog, char *error, ...);

int main(int argc, char **argv) {
    bool push_run_args = false;
    bool should_build = false;
    bool should_run = false;

    NOB_GO_REBUILD_URSELF(argc, argv);

    if (argc < 2) {
        usage(argv[0], "Not enough args. Expected %d or more, got %d", 2, argc);
        return 1;
    }

    if (!read_entire_dir(SRC_DIR, &src_paths))
        return 1;

    // Build
    nob_cc(&build_cmd);

    cmd_append(&build_cmd, "-o", BUILD_DIR MAIN_EXE_NAME);
    cmd_append(&build_cmd, FLAGS);

    for (size_t i = 0; i < src_paths.count; ++i) {
        const char *path = src_paths.items[i];

        if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
            continue;

        char *path_real = temp_sprintf(SRC_DIR "%s", path);
        cmd_append(&build_cmd, path_real);
    }

    // Run
    cmd_append(&run_cmd, BUILD_DIR MAIN_EXE_NAME);

    // Push extra
    for (size_t i = 0; i < (size_t)argc; ++i) {
        char *arg = argv[i];

        // Run arguments
        if (strcmp(arg, "--") == 0) {
            push_run_args = true;
            continue;
        }

        if (push_run_args) {
            cmd_append(&run_cmd, arg);
            continue;
        }

        //
        if (strcmp(arg, "build") == 0)
            should_build = true;

        if (strcmp(arg, "run") == 0)
            should_run = true;
    }

    if (should_build) {
        if (!cmd_run(&build_cmd))
            return 1;
    }

    if (should_run) {
        if (!cmd_run(&run_cmd))
            return 1;
    }

    return 0;
}

void usage(char *prog, char *error, ...) {
    va_list args;
    va_start(args, error);

    nob_log(NOB_INFO, "Usage: %s [CMD] -- [RUN_ARGS]", prog);
    printf("  Commands\n");
    printf("    - build: build the program\n");
    printf("    - run: run the program\n");

    if (error != NULL) {
        char error_real[256];
        vsprintf(error_real, error, args);

        nob_log(NOB_ERROR, "%s", error_real);
    }
    va_end(args);
}
