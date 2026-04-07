#ifndef CFD_DISPATCH_H
#define CFD_DISPATCH_H

#include "../../include/types.h"
#include "../parser/ast.h"

/* Dispatch and execute an AST node */
int cfd_dispatch(cfd_session_t *sess, cfd_ast_node_t *node);

/* Execute a simple command (argv) */
int cfd_dispatch_cmd(cfd_session_t *sess, int argc, char **argv);

/* Execute a pipeline */
int cfd_dispatch_pipeline(cfd_session_t *sess, cfd_ast_node_t **nodes, int count);

/* Look up and run an external program */
int cfd_dispatch_external(cfd_session_t *sess, int argc, char **argv);

#endif /* CFD_DISPATCH_H */
