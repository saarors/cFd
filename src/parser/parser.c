#include "parser.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>

cfd_parser_t *cfd_parser_new(cfd_list_t *tokens) {
    cfd_parser_t *p = CFD_NEW(cfd_parser_t);
    p->tokens = tokens;
    return p;
}

void cfd_parser_free(cfd_parser_t *p) {
    if (!p) return;
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
static cfd_ast_node_t *parse_body(cfd_parser_t *p);
static cfd_ast_node_t *parse_and_or(cfd_parser_t *p);
static cfd_ast_node_t *parse_pipeline(cfd_parser_t *p);
static cfd_ast_node_t *parse_command(cfd_parser_t *p);
static cfd_ast_node_t *parse_for_stmt(cfd_parser_t *p);
static cfd_ast_node_t *parse_while_stmt(cfd_parser_t *p);
static cfd_ast_node_t *parse_if_stmt(cfd_parser_t *p);
static cfd_ast_node_t *parse_if_inner(cfd_parser_t *p);
static cfd_ast_node_t *parse_func_def(cfd_parser_t *p);

/* ── Stop keywords ──────────────────────────────────────────────────── */
/* These terminate a body (do/done/then/fi/else/elif).
   parse_command returns NULL when it sees one, so parse_body stops. */
static int is_stop_keyword(cfd_token_t *tok) {
    if (!tok) return 0;
    return tok->type == TOK_KW_DO   || tok->type == TOK_KW_DONE ||
           tok->type == TOK_KW_THEN || tok->type == TOK_KW_FI   ||
           tok->type == TOK_KW_ELSE || tok->type == TOK_KW_ELIF;
}

/* Returns non-zero if token can be used as a plain word in argv
   (keywords ARE valid arguments to commands like "echo if"). */
static int tok_is_arg(cfd_token_t *tok) {
    if (!tok) return 0;
    return tok->type == TOK_WORD   || tok->type == TOK_STRING ||
           tok->type == TOK_NUMBER || tok->type == TOK_ASSIGN ||
           tok_is_keyword(tok->type); /* keywords valid as args */
}

/* ── Simple command ─────────────────────────────────────────────────── */
static cfd_ast_node_t *parse_command(cfd_parser_t *p) {
    cfd_token_t *tok = peek_tok(p);
    if (!tok || tok->type == TOK_EOF ||
        tok->type == TOK_NEWLINE || tok->type == TOK_SEMI)
        return NULL;

    /* Stop keywords are NOT commands */
    if (is_stop_keyword(tok)) return NULL;

    /* Compound commands */
    switch (tok->type) {
        case TOK_KW_FOR:      return parse_for_stmt(p);
        case TOK_KW_WHILE:
        case TOK_KW_UNTIL:    return parse_while_stmt(p);
        case TOK_KW_IF:       return parse_if_stmt(p);
        case TOK_KW_FUNCTION: return parse_func_def(p);
        default: break;
    }

    /* Check assignment: key=value */
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

    /* ── Simple command: collect argv + redirects ── */
    char *argv[CFD_MAX_ARGS];
    int   argc = 0;
    cfd_redirect_node_t *redirs = NULL, **redir_tail = &redirs;

    /* Check for NAME() { } function definition (lookahead) */
    if (tok->type == TOK_WORD) {
        cfd_token_t *tok2 = (cfd_token_t *)cfd_list_get(p->tokens, p->pos + 1);
        if (tok2 && tok2->type == TOK_LPAREN) {
            cfd_token_t *tok3 = (cfd_token_t *)cfd_list_get(p->tokens, p->pos + 2);
            if (tok3 && tok3->type == TOK_RPAREN) {
                /* NAME() — function definition */
                char *fname = cfd_strdup(tok->value);
                advance_tok(p); /* NAME */
                advance_tok(p); /* (    */
                advance_tok(p); /* )    */
                skip_newlines(p);
                /* Expect { body } */
                cfd_ast_node_t *body = NULL;
                tok = peek_tok(p);
                if (tok && tok->type == TOK_LBRACE) {
                    advance_tok(p);
                    skip_newlines(p);
                    body = parse_body(p);
                    tok = peek_tok(p);
                    if (tok && tok->type == TOK_RBRACE) advance_tok(p);
                } else {
                    body = parse_body(p);
                }
                cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
                n->type      = AST_FUNCTION;
                n->func_name = fname;
                n->body      = body;
                return n;
            }
        }
    }

    while (argc < CFD_MAX_ARGS) {
        tok = peek_tok(p);
        if (!tok || tok->type == TOK_EOF || tok->type == TOK_NEWLINE ||
            tok->type == TOK_SEMI || tok->type == TOK_PIPE ||
            tok->type == TOK_PIPE_ERR || tok->type == TOK_AND ||
            tok->type == TOK_OR || tok->type == TOK_BACKGROUND ||
            tok->type == TOK_RPAREN || tok->type == TOK_RBRACE) break;

        /* Redirects */
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

        if (tok_is_arg(tok)) {
            argv[argc++] = tok->value ? cfd_strdup(tok->value) : cfd_strdup("");
            advance_tok(p);
        } else {
            break;
        }
    }

    if (argc == 0) {
        cfd_redirect_node_t *rd = redirs;
        while (rd) { cfd_redirect_node_t *nx = rd->next; cfd_free(rd->target); cfd_free(rd); rd = nx; }
        return NULL;
    }

    cfd_ast_node_t *node = cfd_ast_cmd(argv, argc);
    for (int i = 0; i < argc; i++) cfd_free(argv[i]);
    node->redirects = redirs;
    return node;
}

/* ── Pipeline ───────────────────────────────────────────────────────── */
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

/* ── And/Or ─────────────────────────────────────────────────────────── */
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

/* ── Body: list of statements that stops at stop keywords ───────────── */
static cfd_ast_node_t *parse_body(cfd_parser_t *p) {
    skip_newlines(p);
    cfd_token_t *tok = peek_tok(p);
    if (!tok || tok->type == TOK_EOF || is_stop_keyword(tok)) return NULL;

    cfd_ast_node_t *left = parse_and_or(p);
    if (!left) return NULL;

    while (true) {
        tok = peek_tok(p);
        if (!tok || tok->type == TOK_EOF || is_stop_keyword(tok)) break;

        if (tok->type == TOK_BACKGROUND) {
            advance_tok(p);
            left = cfd_ast_background(left);
        } else if (tok->type == TOK_SEMI || tok->type == TOK_NEWLINE) {
            advance_tok(p);
            skip_newlines(p);
            tok = peek_tok(p);
            if (!tok || tok->type == TOK_EOF || is_stop_keyword(tok)) break;
            cfd_ast_node_t *right = parse_and_or(p);
            if (!right) break;
            left = cfd_ast_list(left, right);
        } else {
            break;
        }
    }
    return left;
}

/* ── for VAR in WORDS; do BODY; done ───────────────────────────────── */
static cfd_ast_node_t *parse_for_stmt(cfd_parser_t *p) {
    advance_tok(p); /* consume 'for' */
    skip_newlines(p);

    cfd_token_t *var_tok = peek_tok(p);
    if (!var_tok || var_tok->type == TOK_EOF) return NULL;
    char *var_name = cfd_strdup(var_tok->value ? var_tok->value : "");
    advance_tok(p);

    /* Optional 'in wordlist' */
    char *word_list[CFD_MAX_ARGS];
    int   word_count = 0;

    skip_newlines(p);
    cfd_token_t *next = peek_tok(p);
    if (next && next->type == TOK_KW_IN) {
        advance_tok(p); /* consume 'in' */
        while (word_count < CFD_MAX_ARGS) {
            cfd_token_t *w = peek_tok(p);
            if (!w || w->type == TOK_EOF || w->type == TOK_NEWLINE ||
                w->type == TOK_SEMI || w->type == TOK_KW_DO) break;
            if (w->value)
                word_list[word_count++] = cfd_strdup(w->value);
            advance_tok(p);
        }
    }

    /* Skip sep, consume 'do' */
    skip_newlines(p);
    next = peek_tok(p);
    if (next && next->type == TOK_KW_DO) advance_tok(p);
    skip_newlines(p);

    cfd_ast_node_t *body = parse_body(p);

    skip_newlines(p);
    next = peek_tok(p);
    if (next && next->type == TOK_KW_DONE) advance_tok(p);

    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type         = AST_FOR;
    n->var_name     = var_name;
    n->var_list_len = word_count;
    if (word_count > 0) {
        n->var_list = cfd_malloc(word_count * sizeof(char *));
        for (int i = 0; i < word_count; i++) n->var_list[i] = word_list[i];
    }
    n->body = body;
    return n;
}

/* ── while/until CMD; do BODY; done ────────────────────────────────── */
static cfd_ast_node_t *parse_while_stmt(cfd_parser_t *p) {
    cfd_token_type_t kw = peek_tok(p)->type;
    advance_tok(p); /* consume 'while' or 'until' */
    skip_newlines(p);

    cfd_ast_node_t *cond = parse_body(p); /* stops at 'do' */

    skip_newlines(p);
    cfd_token_t *tok = peek_tok(p);
    if (tok && tok->type == TOK_KW_DO) advance_tok(p);
    skip_newlines(p);

    cfd_ast_node_t *body = parse_body(p);

    skip_newlines(p);
    tok = peek_tok(p);
    if (tok && tok->type == TOK_KW_DONE) advance_tok(p);

    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type      = AST_WHILE;
    n->condition = cond;
    n->body      = body;
    /* Repurpose 'background' flag to signal 'until' (inverted condition) */
    if (kw == TOK_KW_UNTIL) n->background = 1;
    return n;
}

/* ── if COND; then BODY; [elif COND; then BODY;]* [else BODY;] fi ─── */
/* Called after 'if' or 'elif' has been consumed */
static cfd_ast_node_t *parse_if_inner(cfd_parser_t *p) {
    skip_newlines(p);

    cfd_ast_node_t *cond = parse_body(p); /* stops at 'then' */

    skip_newlines(p);
    cfd_token_t *tok = peek_tok(p);
    if (tok && tok->type == TOK_KW_THEN) { advance_tok(p); skip_newlines(p); }

    cfd_ast_node_t *body = parse_body(p);
    cfd_ast_node_t *else_body = NULL;

    skip_newlines(p);
    tok = peek_tok(p);

    if (tok && tok->type == TOK_KW_ELIF) {
        advance_tok(p); /* consume 'elif' */
        else_body = parse_if_inner(p); /* recursive; consumes fi */
    } else {
        if (tok && tok->type == TOK_KW_ELSE) {
            advance_tok(p);
            skip_newlines(p);
            else_body = parse_body(p);
            skip_newlines(p);
            tok = peek_tok(p);
        }
        if (tok && tok->type == TOK_KW_FI) advance_tok(p);
    }

    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type      = AST_IF;
    n->condition = cond;
    n->body      = body;
    n->else_body = else_body;
    return n;
}

static cfd_ast_node_t *parse_if_stmt(cfd_parser_t *p) {
    advance_tok(p); /* consume 'if' */
    return parse_if_inner(p);
}

/* ── function NAME { BODY } ─────────────────────────────────────────── */
static cfd_ast_node_t *parse_func_def(cfd_parser_t *p) {
    advance_tok(p); /* consume 'function' */
    skip_newlines(p);

    cfd_token_t *name_tok = peek_tok(p);
    if (!name_tok || name_tok->type == TOK_EOF) return NULL;
    char *fname = cfd_strdup(name_tok->value ? name_tok->value : "");
    advance_tok(p);

    /* Skip optional () */
    cfd_token_t *tok = peek_tok(p);
    if (tok && tok->type == TOK_LPAREN) {
        advance_tok(p);
        tok = peek_tok(p);
        if (tok && tok->type == TOK_RPAREN) advance_tok(p);
    }

    skip_newlines(p);

    /* Parse body: { body } or bare body */
    cfd_ast_node_t *body = NULL;
    tok = peek_tok(p);
    if (tok && tok->type == TOK_LBRACE) {
        advance_tok(p);
        skip_newlines(p);
        body = parse_body(p);
        tok  = peek_tok(p);
        if (tok && tok->type == TOK_RBRACE) advance_tok(p);
    } else {
        body = parse_body(p);
    }

    cfd_ast_node_t *n = CFD_NEW(cfd_ast_node_t);
    n->type      = AST_FUNCTION;
    n->func_name = fname;
    n->body      = body;
    return n;
}

/* ── Top-level list ─────────────────────────────────────────────────── */
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
