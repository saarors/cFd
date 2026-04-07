#include "ast.h"
#include "../utils/mem.h"
#include <string.h>

static cfd_ast_node_t *ast_new(cfd_ast_type_t type) {
    return CFD_NEW(cfd_ast_node_t);
    /* type set by caller */
    (void)type;
}

cfd_ast_node_t *cfd_ast_cmd(char **argv, int argc) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type = AST_CMD;
    n->argc = argc;
    n->argv = cfd_calloc(argc + 1, sizeof(char *));
    for (int i = 0; i < argc; i++) n->argv[i] = cfd_strdup(argv[i]);
    n->argv[argc] = NULL;
    return n;
}

cfd_ast_node_t *cfd_ast_pipeline(cfd_ast_node_t **nodes, int len) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type         = AST_PIPELINE;
    n->pipeline_len = len;
    n->pipeline     = cfd_malloc(len * sizeof(cfd_ast_node_t *));
    memcpy(n->pipeline, nodes, len * sizeof(cfd_ast_node_t *));
    return n;
}

cfd_ast_node_t *cfd_ast_list(cfd_ast_node_t *l, cfd_ast_node_t *r) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type  = AST_LIST;
    n->left  = l;
    n->right = r;
    return n;
}

cfd_ast_node_t *cfd_ast_and(cfd_ast_node_t *l, cfd_ast_node_t *r) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type  = AST_AND;
    n->left  = l;
    n->right = r;
    return n;
}

cfd_ast_node_t *cfd_ast_or(cfd_ast_node_t *l, cfd_ast_node_t *r) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type  = AST_OR;
    n->left  = l;
    n->right = r;
    return n;
}

cfd_ast_node_t *cfd_ast_background(cfd_ast_node_t *node) {
    if (node) node->background = 1;
    return node;
}

cfd_ast_node_t *cfd_ast_assign(const char *key, const char *val) {
    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type       = AST_ASSIGN;
    n->assign_key = cfd_strdup(key);
    n->assign_val = cfd_strdup(val ? val : "");
    return n;
}

void cfd_ast_free(cfd_ast_node_t *node) {
    if (!node) return;
    if (node->argv) {
        for (int i = 0; i < node->argc; i++) cfd_free(node->argv[i]);
        cfd_free(node->argv);
    }
    cfd_free(node->assign_key);
    cfd_free(node->assign_val);
    cfd_free(node->func_name);
    cfd_free(node->var_name);
    if (node->var_list) {
        for (int i = 0; i < node->var_list_len; i++) cfd_free(node->var_list[i]);
        cfd_free(node->var_list);
    }
    if (node->pipeline) {
        for (int i = 0; i < node->pipeline_len; i++) cfd_ast_free(node->pipeline[i]);
        cfd_free(node->pipeline);
    }
    cfd_ast_free(node->left);
    cfd_ast_free(node->right);
    cfd_ast_free(node->condition);
    cfd_ast_free(node->body);
    cfd_ast_free(node->else_body);

    cfd_redirect_node_t *rd = node->redirects;
    while (rd) {
        cfd_redirect_node_t *next = rd->next;
        cfd_free(rd->target);
        cfd_free(rd);
        rd = next;
    }
    cfd_free(node);
}
