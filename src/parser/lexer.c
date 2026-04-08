#include "lexer.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../../include/config.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

cfd_lexer_t *cfd_lexer_new(const char *src) {
    cfd_lexer_t *lex = CFD_NEW(cfd_lexer_t);
    lex->src  = src ? src : "";
    lex->len  = src ? strlen(src) : 0;
    lex->line = 1;
    lex->col  = 1;
    return lex;
}

void cfd_lexer_free(cfd_lexer_t *lex) {
    cfd_free(lex);
}

static char peek(cfd_lexer_t *l) {
    if (l->pos >= l->len) return '\0';
    return l->src[l->pos];
}


static char advance(cfd_lexer_t *l) {
    char c = l->src[l->pos++];
    if (c == '\n') { l->line++; l->col = 1; }
    else l->col++;
    return c;
}

static void skip_whitespace(cfd_lexer_t *l) {
    while (l->pos < l->len) {
        char c = peek(l);
        if (c == ' ' || c == '\t' || c == '\r') advance(l);
        else if (c == '#') { /* comment to end of line */
            while (l->pos < l->len && peek(l) != '\n') advance(l);
        } else break;
    }
}

static char *read_word(cfd_lexer_t *l) {
    size_t start = l->pos;
    while (l->pos < l->len) {
        char c = peek(l);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
            c == '|' || c == '&' || c == ';' || c == '<' || c == '>' ||
            c == '(' || c == ')' || c == '{' || c == '}' || c == '\0') break;
        if (c == '\\' && l->pos + 1 < l->len) { advance(l); advance(l); continue; }
        advance(l);
    }
    return cfd_strndup(l->src + start, l->pos - start);
}

static char *read_quoted(cfd_lexer_t *l, char q) {
    advance(l); /* skip opening quote */
    char buf[CFD_BUF_LARGE];
    size_t bi = 0;
    while (l->pos < l->len && peek(l) != q) {
        char c = peek(l);
        if (c == '\\' && q == '"') {
            advance(l);
            c = peek(l);
            switch (c) {
                case 'n': buf[bi++] = '\n'; break;
                case 't': buf[bi++] = '\t'; break;
                case 'r': buf[bi++] = '\r'; break;
                case '\\': buf[bi++] = '\\'; break;
                default:   buf[bi++] = c; break;
            }
            advance(l);
        } else {
            buf[bi++] = advance(l);
        }
        if (bi >= sizeof(buf) - 1) break;
    }
    if (peek(l) == q) advance(l); /* closing quote */
    buf[bi] = '\0';
    return cfd_strdup(buf);
}

cfd_token_t *cfd_lexer_next(cfd_lexer_t *lex) {
    skip_whitespace(lex);
    if (lex->pos >= lex->len)
        return cfd_token_new(TOK_EOF, NULL, lex->line, lex->col);

    size_t line = lex->line, col = lex->col;
    char c = peek(lex);

    /* newline */
    if (c == '\n') { advance(lex); return cfd_token_new(TOK_NEWLINE, "\n", line, col); }

    /* semicolon */
    if (c == ';') { advance(lex); return cfd_token_new(TOK_SEMI, ";", line, col); }

    /* parens/braces */
    if (c == '(') { advance(lex); return cfd_token_new(TOK_LPAREN, "(", line, col); }
    if (c == ')') { advance(lex); return cfd_token_new(TOK_RPAREN, ")", line, col); }
    if (c == '{') { advance(lex); return cfd_token_new(TOK_LBRACE, "{", line, col); }
    if (c == '}') { advance(lex); return cfd_token_new(TOK_RBRACE, "}", line, col); }

    /* pipe / |& / || */
    if (c == '|') {
        advance(lex);
        if (peek(lex) == '&') { advance(lex); return cfd_token_new(TOK_PIPE_ERR, "|&", line, col); }
        if (peek(lex) == '|') { advance(lex); return cfd_token_new(TOK_OR,       "||", line, col); }
        return cfd_token_new(TOK_PIPE, "|", line, col);
    }

    /* & / && */
    if (c == '&') {
        advance(lex);
        if (peek(lex) == '&') { advance(lex); return cfd_token_new(TOK_AND, "&&", line, col); }
        if (peek(lex) == '>') { advance(lex); return cfd_token_new(TOK_REDIR_ERR_OUT, "&>", line, col); }
        return cfd_token_new(TOK_BACKGROUND, "&", line, col);
    }

    /* redirects */
    if (c == '<') {
        advance(lex);
        if (peek(lex) == '<') { advance(lex); return cfd_token_new(TOK_HEREDOC, "<<", line, col); }
        return cfd_token_new(TOK_REDIR_IN, "<", line, col);
    }
    if (c == '>') {
        advance(lex);
        if (peek(lex) == '>') { advance(lex); return cfd_token_new(TOK_REDIR_APPEND, ">>", line, col); }
        return cfd_token_new(TOK_REDIR_OUT, ">", line, col);
    }

    /* quoted strings */
    if (c == '"' || c == '\'') {
        char *s = read_quoted(lex, c);
        cfd_token_t *tok = cfd_token_new(TOK_STRING, s, line, col);
        cfd_free(s);
        return tok;
    }

    /* word / keyword / assignment */
    {
        char *w = read_word(lex);
        /* check for assignment: key=value */
        char *eq = strchr(w, '=');
        if (eq && eq != w) {
            cfd_token_t *tok = cfd_token_new(TOK_ASSIGN, w, line, col);
            cfd_free(w);
            return tok;
        }
        /* check keywords */
        static const struct { const char *kw; cfd_token_type_t type; } kwtab[] = {
            {"for",      TOK_KW_FOR},
            {"in",       TOK_KW_IN},
            {"do",       TOK_KW_DO},
            {"done",     TOK_KW_DONE},
            {"if",       TOK_KW_IF},
            {"then",     TOK_KW_THEN},
            {"elif",     TOK_KW_ELIF},
            {"else",     TOK_KW_ELSE},
            {"fi",       TOK_KW_FI},
            {"while",    TOK_KW_WHILE},
            {"until",    TOK_KW_UNTIL},
            {"function", TOK_KW_FUNCTION},
            {NULL, TOK_WORD}
        };
        for (int ki = 0; kwtab[ki].kw; ki++) {
            if (strcmp(w, kwtab[ki].kw) == 0) {
                cfd_token_t *tok = cfd_token_new(kwtab[ki].type, w, line, col);
                cfd_free(w);
                return tok;
            }
        }
        /* check numeric */
        bool is_num = true;
        for (char *p = w; *p; p++) if (!isdigit((unsigned char)*p)) { is_num = false; break; }
        cfd_token_t *tok = cfd_token_new(is_num ? TOK_NUMBER : TOK_WORD, w, line, col);
        cfd_free(w);
        return tok;
    }
}

cfd_list_t *cfd_lexer_tokenize(const char *src) {
    cfd_lexer_t *lex = cfd_lexer_new(src);
    cfd_list_t *list = cfd_list_new((cfd_list_free_fn)cfd_token_free);
    cfd_token_t *tok;
    do {
        tok = cfd_lexer_next(lex);
        cfd_list_push_back(list, tok);
    } while (tok->type != TOK_EOF);
    cfd_lexer_free(lex);
    return list;
}

bool cfd_lexer_has_error(const cfd_lexer_t *lex) {
    return lex->error[0] != '\0';
}

const char *cfd_lexer_error(const cfd_lexer_t *lex) {
    return lex->error;
}
