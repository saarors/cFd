#ifndef CFD_AST_H
#define CFD_AST_H

#include <stddef.h>

typedef enum {
    AST_CMD,        /* simple command */
    AST_PIPELINE,   /* cmd | cmd | ... */
    AST_LIST,       /* cmd ; cmd */
    AST_AND,        /* cmd && cmd */
    AST_OR,         /* cmd || cmd */
    AST_BACKGROUND, /* cmd & */
    AST_ASSIGN,     /* var=val */
    AST_SUBSHELL,   /* (cmd) */
    AST_IF,         /* if/then/elif/else/fi */
    AST_WHILE,      /* while/do/done */
    AST_FOR,        /* for/in/do/done */
    AST_FUNCTION,   /* name() { body } */
    AST_REDIRECT    /* wrapper with redirections */
} cfd_ast_type_t;

typedef struct cfd_redirect_node {
    int   fd;
    char *target;
    int   type; /* cfd_redir_type_t */
    struct cfd_redirect_node *next;
} cfd_redirect_node_t;

typedef struct cfd_ast_node {
    cfd_ast_type_t type;

    /* AST_CMD */
    char **argv;
    int    argc;

    /* AST_ASSIGN */
    char *assign_key;
    char *assign_val;

    /* children for compound nodes */
    struct cfd_ast_node *left;
    struct cfd_ast_node *right;

    /* for pipeline: array of nodes */
    struct cfd_ast_node **pipeline;
    int                   pipeline_len;

    /* redirections attached to this node */
    cfd_redirect_node_t *redirects;

    /* background flag */
    int background;

    /* for if/while/for */
    struct cfd_ast_node *condition;
    struct cfd_ast_node *body;
    struct cfd_ast_node *else_body;

    /* for loop variable */
    char  *var_name;
    char **var_list;
    int    var_list_len;

    /* function name */
    char *func_name;
} cfd_ast_node_t;

cfd_ast_node_t *cfd_ast_cmd(char **argv, int argc);
cfd_ast_node_t *cfd_ast_pipeline(cfd_ast_node_t **nodes, int len);
cfd_ast_node_t *cfd_ast_list(cfd_ast_node_t *left, cfd_ast_node_t *right);
cfd_ast_node_t *cfd_ast_and(cfd_ast_node_t *l, cfd_ast_node_t *r);
cfd_ast_node_t *cfd_ast_or(cfd_ast_node_t *l, cfd_ast_node_t *r);
cfd_ast_node_t *cfd_ast_background(cfd_ast_node_t *node);
cfd_ast_node_t *cfd_ast_assign(const char *key, const char *val);

void cfd_ast_free(cfd_ast_node_t *node);

#endif /* CFD_AST_H */
