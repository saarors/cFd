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
    /* Simple sequential pipeline: use popen-style temp files */
    int ret = 0;
    char *prev_out = NULL;
    for (int i = 0; i < count; i++) {
        if (prev_out) {
            /* redirect stdin from prev_out */
            freopen(prev_out, "r", stdin);
            cfd_free(prev_out); prev_out = NULL;
        }
        if (i < count - 1) {
            /* redirect stdout to temp file */
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "cfd_pipe_%d.tmp", i);
            prev_out = cfd_strdup(tmp);
            freopen(tmp, "w", stdout);
        }
        ret = cfd_dispatch(sess, nodes[i]);
        fflush(stdout);
    }
    /* restore stdio */
#ifdef _WIN32
    freopen("CON", "r", stdin);
    freopen("CON", "w", stdout);
#else
    freopen("/dev/tty", "r", stdin);
    freopen("/dev/tty", "w", stdout);
#endif
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
