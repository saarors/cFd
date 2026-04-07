#include "dispatch.h"
#include "registry.h"
#include "../core/session.h"
#include "../scripting/variable.h"
#include "../scripting/function.h"
#include "../scripting/control.h"
#include "../io/redirect.h"
#include "../utils/mem.h"
#include "../utils/path.h"
#include "../utils/str_utils.h"
#include "../platform/platform.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#  include <io.h>
#  define CFD_DUP(fd)         _dup(fd)
#  define CFD_DUP2(fd,fd2)    _dup2(fd,fd2)
#  define CFD_CLOSE(fd)       _close(fd)
#  define CFD_PIPE(fds,sz,fl) _pipe(fds,sz,fl)
#else
#  include <unistd.h>
#  define CFD_DUP(fd)         dup(fd)
#  define CFD_DUP2(fd,fd2)    dup2(fd,fd2)
#  define CFD_CLOSE(fd)       close(fd)
static inline int cfd_unix_pipe(int fds[2]) { return pipe(fds); }
#  define CFD_PIPE(fds,sz,fl) cfd_unix_pipe(fds)
#endif
#ifdef _WIN32
#  include <fcntl.h>
#endif

/* Find external program in PATH */
static char *find_in_path(const char *name) {
    if (cfd_path_is_absolute(name)) {
        if (cfd_path_exists(name)) return cfd_strdup(name);
        return NULL;
    }
    char *path_env = cfd_platform_getenv("PATH");
    if (!path_env) return NULL;
    int n;
    char **dirs = cfd_path_split_dirs(path_env, &n);
    cfd_free(path_env);
    char *found = NULL;
    for (int i = 0; i < n && !found; i++) {
        char *full = cfd_path_join(dirs[i], name);
#ifdef _WIN32
        /* Try with .exe extension */
        char *fullexe = cfd_sprintf("%s.exe", full);
        if (cfd_path_exists(fullexe)) { cfd_free(full); found = fullexe; break; }
        cfd_free(fullexe);
        char *fullcmd = cfd_sprintf("%s.cmd", full);
        if (cfd_path_exists(fullcmd)) { cfd_free(full); found = fullcmd; break; }
        cfd_free(fullcmd);
#endif
        if (cfd_path_exists(full)) { found = full; break; }
        cfd_free(full);
    }
    cfd_strfreev(dirs);
    return found;
}

int cfd_dispatch_external(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    char *prog = find_in_path(argv[0]);
    if (!prog) {
        fprintf(stderr, "cFd: %s: command not found\n", argv[0]);
        return 127;
    }
    int ret = cfd_platform_run_program(prog, argv, NULL);
    cfd_free(prog);
    return ret;
}

/* Expand variables in all argv */
static char **expand_argv(cfd_session_t *sess, int argc, char **argv) {
    char **out = cfd_malloc((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; i++)
        out[i] = cfd_var_expand(sess->vars, argv[i]);
    out[argc] = NULL;
    return out;
}

int cfd_dispatch_cmd(cfd_session_t *sess, int argc, char **argv) {
    if (argc == 0 || !argv[0]) return 0;

    /* Expand variables */
    char **eargv = expand_argv(sess, argc, argv);
    int ret = 0;

    /* Check alias */
    const char *alias_val = cfd_session_get_alias(sess, eargv[0]);
    if (alias_val) {
        char *expanded = cfd_sprintf("%s", alias_val);
        /* Build new argv: alias expansion + original args */
        int alias_argc;
        char **alias_argv = cfd_strsplit(expanded, " ", &alias_argc);
        cfd_free(expanded);
        int total = alias_argc + argc - 1;
        char **merged = cfd_malloc((total + 1) * sizeof(char *));
        for (int i = 0; i < alias_argc; i++) merged[i] = alias_argv[i];
        for (int i = 1; i < argc;       i++) merged[alias_argc + i - 1] = eargv[i];
        merged[total] = NULL;
        /* Recursion with merged (shallow, no free aliases here) */
        ret = cfd_dispatch_cmd(sess, total, merged);
        cfd_free(alias_argv); /* not strfreev, strings are still referenced */
        cfd_free(merged);
        for (int i = 0; i < argc; i++) cfd_free(eargv[i]);
        cfd_free(eargv);
        return ret;
    }

    /* Check built-in */
    const cfd_command_t *cmd = cfd_registry_find(g_registry, eargv[0]);
    if (cmd) {
        ret = cmd->handler(sess, argc, eargv);
        for (int i = 0; i < argc; i++) cfd_free(eargv[i]);
        cfd_free(eargv);
        return ret;
    }

    /* Check user-defined function */
    cfd_function_t *fn = cfd_func_get(sess->funcs, eargv[0]);
    if (fn) {
        ret = cfd_func_call(sess, fn, argc, eargv);
        for (int i = 0; i < argc; i++) cfd_free(eargv[i]);
        cfd_free(eargv);
        return ret;
    }

    /* External */
    ret = cfd_dispatch_external(sess, argc, eargv);
    for (int i = 0; i < argc; i++) cfd_free(eargv[i]);
    cfd_free(eargv);
    return ret;
}

int cfd_dispatch_pipeline(cfd_session_t *sess, cfd_ast_node_t **nodes, int count) {
    if (count == 1) return cfd_dispatch(sess, nodes[0]);

    /* Save original stdin/stdout file descriptors */
    int saved_stdin  = CFD_DUP(0);
    int saved_stdout = CFD_DUP(1);
    if (saved_stdin < 0 || saved_stdout < 0) {
        /* fallback: run sequentially without pipes */
        int ret = 0;
        for (int i = 0; i < count; i++) ret = cfd_dispatch(sess, nodes[i]);
        return ret;
    }

    int ret = 0;
    int prev_read = -1; /* read end of the previous pipe */

    for (int i = 0; i < count; i++) {
        int pipefd[2] = {-1, -1};
        int need_pipe = (i < count - 1);

        if (need_pipe) {
#ifdef _WIN32
            if (CFD_PIPE(pipefd, 65536, _O_BINARY) != 0) {
                /* can't create pipe; abort */
                break;
            }
#else
            if (CFD_PIPE(pipefd, 0, 0) != 0) {
                break;
            }
#endif
        }

        /* Set stdin from previous pipe's read end */
        if (prev_read >= 0) {
            CFD_DUP2(prev_read, 0);
            CFD_CLOSE(prev_read);
            prev_read = -1;
        } else if (i > 0) {
            /* restore stdin for first stage already handled above */
        }

        /* Set stdout to current pipe's write end */
        if (need_pipe) {
            CFD_DUP2(pipefd[1], 1);
            CFD_CLOSE(pipefd[1]);
            prev_read = pipefd[0];
        }

        fflush(stdout);
        ret = cfd_dispatch(sess, nodes[i]);
        fflush(stdout);

        /* Restore stdout for next iteration (or final) */
        CFD_DUP2(saved_stdout, 1);

        /* Restore stdin for next iteration */
        if (i < count - 1 && prev_read < 0) {
            /* already consumed */
        }
    }

    /* If there's a leftover pipe read end, close it */
    if (prev_read >= 0) CFD_CLOSE(prev_read);

    /* Restore original stdin/stdout */
    CFD_DUP2(saved_stdin,  0);
    CFD_DUP2(saved_stdout, 1);
    CFD_CLOSE(saved_stdin);
    CFD_CLOSE(saved_stdout);

    return ret;
}

int cfd_dispatch(cfd_session_t *sess, cfd_ast_node_t *node) {
    if (!node) return 0;
    int ret = 0;
    cfd_redirect_ctx_t rctx = {0};

    switch (node->type) {
        case AST_CMD: {
            if (node->redirects) cfd_redirect_apply(node->redirects, &rctx);
            ret = cfd_dispatch_cmd(sess, node->argc, node->argv);
            if (node->redirects) cfd_redirect_restore(&rctx);
            break;
        }
        case AST_ASSIGN: {
            cfd_session_set_var(sess, node->assign_key, node->assign_val);
            ret = 0;
            break;
        }
        case AST_PIPELINE: {
            ret = cfd_dispatch_pipeline(sess, node->pipeline, node->pipeline_len);
            break;
        }
        case AST_LIST: {
            ret = cfd_dispatch(sess, node->left);
            if (node->right) ret = cfd_dispatch(sess, node->right);
            break;
        }
        case AST_AND: {
            ret = cfd_dispatch(sess, node->left);
            if (ret == 0) ret = cfd_dispatch(sess, node->right);
            break;
        }
        case AST_OR: {
            ret = cfd_dispatch(sess, node->left);
            if (ret != 0) ret = cfd_dispatch(sess, node->right);
            break;
        }
        case AST_BACKGROUND: {
            /* On Windows, just run synchronously for now */
            ret = cfd_dispatch(sess, node->left ? node->left : node);
            break;
        }
        case AST_IF: {
            cfd_ctrl_state_t ctrl = {CFD_CTRL_NORMAL, 0};
            ret = cfd_ctrl_eval_if(sess, node, &ctrl);
            break;
        }
        case AST_WHILE: {
            cfd_ctrl_state_t ctrl = {CFD_CTRL_NORMAL, 0};
            ret = cfd_ctrl_eval_while(sess, node, &ctrl);
            break;
        }
        case AST_FOR: {
            cfd_ctrl_state_t ctrl = {CFD_CTRL_NORMAL, 0};
            ret = cfd_ctrl_eval_for(sess, node, &ctrl);
            break;
        }
        case AST_FUNCTION: {
            cfd_func_define(sess->funcs, node->func_name, node->body);
            node->body = NULL; /* ownership transferred */
            ret = 0;
            break;
        }
        default: break;
    }

    sess->last_exit = ret;
    return ret;
}
