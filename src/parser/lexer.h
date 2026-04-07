#ifndef CFD_LEXER_H
#define CFD_LEXER_H

#include "token.h"
#include "../utils/list.h"

typedef struct cfd_lexer {
    const char *src;
    size_t      pos;
    size_t      len;
    size_t      line;
    size_t      col;
    char        error[512];
} cfd_lexer_t;

cfd_lexer_t  *cfd_lexer_new(const char *src);
void          cfd_lexer_free(cfd_lexer_t *lex);

cfd_token_t  *cfd_lexer_next(cfd_lexer_t *lex);
cfd_list_t   *cfd_lexer_tokenize(const char *src);

bool          cfd_lexer_has_error(const cfd_lexer_t *lex);
const char   *cfd_lexer_error(const cfd_lexer_t *lex);

#endif /* CFD_LEXER_H */
