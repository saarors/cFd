#ifndef CFD_COMMAND_H
#define CFD_COMMAND_H

#include "../../include/types.h"

typedef struct cfd_command {
    const char   *name;
    const char   *usage;
    const char   *description;
    const char   *category;
    cfd_cmd_fn_t  handler;
    int           min_args;
    int           max_args;  /* -1 = unlimited */
} cfd_command_t;

/* Convenience: check arg count and print usage on mismatch */
int cfd_cmd_check_args(const cfd_command_t *cmd, int argc);

#endif /* CFD_COMMAND_H */
