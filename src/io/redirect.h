#ifndef CFD_REDIRECT_H
#define CFD_REDIRECT_H

#include "../../include/types.h"
#include "../parser/ast.h"
#include <stdio.h>

typedef struct cfd_saved_fd {
    int orig_fd;
    int saved_fd;
} cfd_saved_fd_t;

typedef struct cfd_redirect_ctx {
    cfd_saved_fd_t saved[16];
    int            count;
} cfd_redirect_ctx_t;

/* Apply redirections from an AST node. Saves original fds for later restore. */
int  cfd_redirect_apply(cfd_redirect_node_t *redirs, cfd_redirect_ctx_t *ctx);

/* Restore saved file descriptors */
void cfd_redirect_restore(cfd_redirect_ctx_t *ctx);

#endif /* CFD_REDIRECT_H */
