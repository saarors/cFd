#include "token.h"
#include "../utils/mem.h"
#include <string.h>

cfd_token_t *cfd_token_new(cfd_token_type_t type, const char *value, size_t line, size_t col) {
    cfd_token_t *tok = CFD_NEW(cfd_token_t);
    tok->type  = type;
    tok->value = value ? cfd_strdup(value) : NULL;
    tok->line  = line;
    tok->col   = col;
    return tok;
}

void cfd_token_free(cfd_token_t *tok) {
    if (!tok) return;
    cfd_free(tok->value);
    cfd_free(tok);
}

const char *cfd_token_type_name(cfd_token_type_t type) {
    switch (type) {
        case TOK_WORD:         return "WORD";
        case TOK_STRING:       return "STRING";
        case TOK_NUMBER:       return "NUMBER";
        case TOK_ASSIGN:       return "ASSIGN";
        case TOK_PIPE:         return "PIPE";
        case TOK_PIPE_ERR:     return "PIPE_ERR";
        case TOK_REDIR_IN:     return "REDIR_IN";
        case TOK_REDIR_OUT:    return "REDIR_OUT";
        case TOK_REDIR_APPEND: return "REDIR_APPEND";
        case TOK_REDIR_ERR:    return "REDIR_ERR";
        case TOK_REDIR_ERR_OUT:return "REDIR_ERR_OUT";
        case TOK_HEREDOC:      return "HEREDOC";
        case TOK_SEMI:         return "SEMI";
        case TOK_AND:          return "AND";
        case TOK_OR:           return "OR";
        case TOK_BACKGROUND:   return "BACKGROUND";
        case TOK_LPAREN:       return "LPAREN";
        case TOK_RPAREN:       return "RPAREN";
        case TOK_LBRACE:       return "LBRACE";
        case TOK_RBRACE:       return "RBRACE";
        case TOK_NEWLINE:      return "NEWLINE";
        case TOK_EOF:          return "EOF";
        case TOK_ERROR:        return "ERROR";
        default:               return "UNKNOWN";
    }
}
