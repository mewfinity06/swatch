#include <stddef.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

#include "../inc/utils.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0], "Not enough args. Expected %d or more, got %d", 2, argc);
    return 1;
  }

  for (size_t i = 0; i < (size_t)argc; ++i) {
    nob_log(NOB_INFO, "(%2zu): %s", i, argv[i]);
  }
}
