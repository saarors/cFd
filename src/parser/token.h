#ifndef CFD_TOKEN_H
#define CFD_TOKEN_H

#include <stddef.h>

typedef enum {
    TOK_WORD = 0,       /* plain word / command name */
    TOK_STRING,         /* quoted string */
    TOK_NUMBER,         /* numeric literal */
    TOK_ASSIGN,         /* key=value */
    TOK_PIPE,           /* | */
    TOK_PIPE_ERR,       /* |& */
    TOK_REDIR_IN,       /* < */
    TOK_REDIR_OUT,      /* > */
    TOK_REDIR_APPEND,   /* >> */
    TOK_REDIR_ERR,      /* 2> */
    TOK_REDIR_ERR_OUT,  /* 2>&1 / &> */
    TOK_HEREDOC,        /* << */
    TOK_SEMI,           /* ; */
    TOK_AND,            /* && */
    TOK_OR,             /* || */
    TOK_BACKGROUND,     /* & */
    TOK_LPAREN,         /* ( */
    TOK_RPAREN,         /* ) */
    TOK_LBRACE,         /* { */
    TOK_RBRACE,         /* } */
    TOK_NEWLINE,
    TOK_EOF,
    TOK_ERROR
} cfd_token_type_t;

typedef struct cfd_token {
    cfd_token_type_t type;
    char            *value;
    size_t           line;
    size_t           col;
} cfd_token_t;

cfd_token_t *cfd_token_new(cfd_token_type_t type, const char *value, size_t line, size_t col);
void         cfd_token_free(cfd_token_t *tok);
const char  *cfd_token_type_name(cfd_token_type_t type);

#endif /* CFD_TOKEN_H */
