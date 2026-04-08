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
    TOK_ERROR,
    /* ── Shell keywords ────────────────────────────────────────────────── */
    TOK_KW_FOR,         /* for   */
    TOK_KW_IN,          /* in    */
    TOK_KW_DO,          /* do    */
    TOK_KW_DONE,        /* done  */
    TOK_KW_IF,          /* if    */
    TOK_KW_THEN,        /* then  */
    TOK_KW_ELIF,        /* elif  */
    TOK_KW_ELSE,        /* else  */
    TOK_KW_FI,          /* fi    */
    TOK_KW_WHILE,       /* while */
    TOK_KW_UNTIL,       /* until */
    TOK_KW_FUNCTION     /* function */
} cfd_token_type_t;

/* Returns true if tok is a shell keyword */
static inline int tok_is_keyword(cfd_token_type_t t) {
    return t >= TOK_KW_FOR && t <= TOK_KW_FUNCTION;
}

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
