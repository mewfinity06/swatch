#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"

Cmd cmd = {0};

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  for (size_t i = 0; i < argc; ++i) {
    char *arg = argv[i];
    if (strcmp(arg, "build") == 0) {
      if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

      nob_cc(&cmd);
      nob_cc_flags(&cmd);
      nob_cc_output(&cmd, BUILD_DIR "swatch");
      nob_cc_inputs(&cmd, SRC_DIR "main.c");
      if (!cmd_run(&cmd))
        return 1;
    }

    if (strcmp(arg, "run") == 0) {
      cmd_append(&cmd, BUILD_DIR "swatch");
      if (!cmd_run(&cmd))
        return 1;
    }
  }

  return 0;
}
