#ifndef CFD_SESSION_H
#define CFD_SESSION_H

#include "../../include/types.h"
#include "../../include/config.h"
#include "../scripting/variable.h"
#include "../scripting/function.h"
#include "../utils/hash.h"
#include <stdbool.h>

typedef struct cfd_job_entry {
    int   pid;
    char  cmd[CFD_MAX_NAME];
    int   status; /* 0=running, 1=stopped, 2=done */
} cfd_job_entry_t;

typedef struct cfd_session {
    /* Current working directory */
    char *cwd;
    char *prev_cwd;

    /* Variables and functions */
    cfd_var_store_t  *vars;
    cfd_func_store_t *funcs;

    /* Aliases: name -> value (heap strings) */
    cfd_hash_t       *aliases;

    /* Job table */
    cfd_job_entry_t   jobs[CFD_MAX_JOBS];
    int               job_count;

    /* Last exit code */
    int  last_exit;

    /* Running flag */
    bool running;
    int  exit_code;

    /* Terminal PID */
    int pid;
} cfd_session_t;

cfd_session_t *cfd_session_new(void);
void           cfd_session_free(cfd_session_t *sess);

/* Variable helpers */
void        cfd_session_set_var(cfd_session_t *sess, const char *name, const char *val);
const char *cfd_session_get_var(cfd_session_t *sess, const char *name);

/* Alias helpers */
const char *cfd_session_get_alias(cfd_session_t *sess, const char *name);

/* Execute a parsed AST node (defined in dispatch.c) */
int cfd_session_exec_node(cfd_session_t *sess, cfd_ast_node_t *node);

/* Execute a raw string */
int cfd_session_exec_string(cfd_session_t *sess, const char *line);

#endif /* CFD_SESSION_H */
