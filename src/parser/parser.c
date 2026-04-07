#include "parser.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include <string.h>
#include <stdio.h>

cfd_parser_t *cfd_parser_new(cfd_list_t *tokens) {
    cfd_parser_t *p = CFD_NEW(cfd_parser_t);
    p->tokens = tokens;
    return p;
}

void cfd_parser_free(cfd_parser_t *p) {
    if (!p) return;
    /* tokens list is owned by caller */
    cfd_free(p);
}

static cfd_token_t *peek_tok(cfd_parser_t *p) {
    return (cfd_token_t *)cfd_list_get(p->tokens, p->pos);
}

static cfd_token_t *advance_tok(cfd_parser_t *p) {
    cfd_token_t *tok = (cfd_token_t *)cfd_list_get(p->tokens, p->pos);
    if (tok && tok->type != TOK_EOF) p->pos++;
    return tok;
}

static void skip_newlines(cfd_parser_t *p) {
    while (true) {
        cfd_token_t *tok = peek_tok(p);
        if (!tok || (tok->type != TOK_NEWLINE && tok->type != TOK_SEMI)) break;
        advance_tok(p);
    }
}

/* Forward declarations */
static cfd_ast_node_t *parse_list(cfd_parser_t *p);
static cfd_ast_node_t *parse_and_or(cfd_parser_t *p);
static cfd_ast_node_t *parse_pipeline(cfd_parser_t *p);
static cfd_ast_node_t *parse_command(cfd_parser_t *p);

static cfd_ast_node_t *parse_command(cfd_parser_t *p) {
    cfd_token_t *tok = peek_tok(p);
    if (!tok || tok->type == TOK_EOF || tok->type == TOK_NEWLINE || tok->type == TOK_SEMI)
        return NULL;

    /* Check assignment */
    if (tok->type == TOK_ASSIGN) {
        advance_tok(p);
        char *eq = strchr(tok->value, '=');
        if (eq) {
            *eq = '\0';
            cfd_ast_node_t *n = cfd_ast_assign(tok->value, eq + 1);
            *eq = '=';
            return n;
        }
    }

    /* Build argv */
    char *argv[CFD_MAX_ARGS];
    int   argc = 0;
    cfd_redirect_node_t *redirs = NULL, **redir_tail = &redirs;

    while (argc < CFD_MAX_ARGS) {
        tok = peek_tok(p);
        if (!tok || tok->type == TOK_EOF || tok->type == TOK_NEWLINE ||
            tok->type == TOK_SEMI || tok->type == TOK_PIPE ||
            tok->type == TOK_PIPE_ERR || tok->type == TOK_AND ||
            tok->type == TOK_OR || tok->type == TOK_BACKGROUND ||
            tok->type == TOK_RPAREN || tok->type == TOK_RBRACE) break;

        /* handle redirects */
        if (tok->type == TOK_REDIR_IN || tok->type == TOK_REDIR_OUT ||
            tok->type == TOK_REDIR_APPEND || tok->type == TOK_REDIR_ERR ||
            tok->type == TOK_REDIR_ERR_OUT || tok->type == TOK_HEREDOC) {
            int rtype = tok->type;
            advance_tok(p);
            cfd_token_t *target = peek_tok(p);
            if (!target || target->type == TOK_EOF) break;
            advance_tok(p);
            cfd_redirect_node_t *rd = CFD_NEW(cfd_redirect_node_t);
            rd->type   = rtype;
            rd->target = cfd_strdup(target->value ? target->value : "");
            *redir_tail = rd;
            redir_tail  = &rd->next;
            continue;
        }

        if (tok->type == TOK_WORD || tok->type == TOK_STRING ||
            tok->type == TOK_NUMBER || tok->type == TOK_ASSIGN) {
            argv[argc++] = tok->value ? cfd_strdup(tok->value) : cfd_strdup("");
            advance_tok(p);
        } else {
            break;
        }
    }

    if (argc == 0) { /* clean up redirects and return NULL */
        cfd_redirect_node_t *rd = redirs;
        while (rd) { cfd_redirect_node_t *nx = rd->next; cfd_free(rd->target); cfd_free(rd); rd = nx; }
        return NULL;
    }

    cfd_ast_node_t *node = cfd_ast_cmd(argv, argc);
    for (int i = 0; i < argc; i++) cfd_free(argv[i]);
    node->redirects = redirs;
    return node;
}

static cfd_ast_node_t *parse_pipeline(cfd_parser_t *p) {
    cfd_ast_node_t *cmds[CFD_MAX_PIPE_DEPTH];
    int n = 0;
    cfd_ast_node_t *first = parse_command(p);
    if (!first) return NULL;
    cmds[n++] = first;

    while (n < CFD_MAX_PIPE_DEPTH) {
        cfd_token_t *tok = peek_tok(p);
        if (!tok || (tok->type != TOK_PIPE && tok->type != TOK_PIPE_ERR)) break;
        advance_tok(p);
        skip_newlines(p);
        cfd_ast_node_t *next = parse_command(p);
        if (!next) break;
        cmds[n++] = next;
    }

    if (n == 1) return cmds[0];
    return cfd_ast_pipeline(cmds, n);
}

static cfd_ast_node_t *parse_and_or(cfd_parser_t *p) {
    cfd_ast_node_t *left = parse_pipeline(p);
    if (!left) return NULL;

    while (true) {
        cfd_token_t *tok = peek_tok(p);
        if (!tok || (tok->type != TOK_AND && tok->type != TOK_OR)) break;
        cfd_token_type_t op = tok->type;
        advance_tok(p);
        skip_newlines(p);
        cfd_ast_node_t *right = parse_pipeline(p);
        if (!right) break;
        left = (op == TOK_AND) ? cfd_ast_and(left, right) : cfd_ast_or(left, right);
    }
    return left;
}

static cfd_ast_node_t *parse_list(cfd_parser_t *p) {
    skip_newlines(p);
    cfd_ast_node_t *left = parse_and_or(p);
    if (!left) return NULL;

    while (true) {
        cfd_token_t *tok = peek_tok(p);
        if (!tok || tok->type == TOK_EOF) break;

        if (tok->type == TOK_BACKGROUND) {
            advance_tok(p);
            left = cfd_ast_background(left);
        } else if (tok->type == TOK_SEMI || tok->type == TOK_NEWLINE) {
            advance_tok(p);
            skip_newlines(p);
            tok = peek_tok(p);
            if (!tok || tok->type == TOK_EOF) break;
            cfd_ast_node_t *right = parse_and_or(p);
            if (!right) break;
            left = cfd_ast_list(left, right);
        } else {
            break;
        }
    }
    return left;
}

cfd_ast_node_t *cfd_parser_parse(cfd_parser_t *p) {
    return parse_list(p);
}

cfd_ast_node_t *cfd_parse(const char *input) {
    if (!input || !*input) return NULL;
    cfd_list_t   *toks = cfd_lexer_tokenize(input);
    cfd_parser_t *p    = cfd_parser_new(toks);
    cfd_ast_node_t *ast = cfd_parser_parse(p);
    cfd_parser_free(p);
    cfd_list_free(toks);
    return ast;
}

bool cfd_parser_has_error(const cfd_parser_t *p) { return p && p->error[0]; }
const char *cfd_parser_error(const cfd_parser_t *p) { return p ? p->error : ""; }
