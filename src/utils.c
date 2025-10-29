#include <stdarg.h>
#include <stdio.h>

#include "../inc/utils.h"
#include "../nob.h"

void usage(char *prog, char *error, ...) {
  va_list args;
  va_start(args, error);

  nob_log(NOB_INFO, "USAGE: %s", prog);
  if (error != NULL) {
    char error_real[256];
    vsprintf(error_real, error, args);
    nob_log(NOB_ERROR, "Error: %s\n", error_real);
  }

  va_end(args);
}
