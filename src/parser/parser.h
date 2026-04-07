#ifndef CFD_PARSER_H
#define CFD_PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct cfd_parser {
    cfd_list_t  *tokens;
    size_t       pos;
    char         error[512];
} cfd_parser_t;

cfd_parser_t   *cfd_parser_new(cfd_list_t *tokens);
void            cfd_parser_free(cfd_parser_t *p);

cfd_ast_node_t *cfd_parse(const char *input);
cfd_ast_node_t *cfd_parser_parse(cfd_parser_t *p);

bool            cfd_parser_has_error(const cfd_parser_t *p);
const char     *cfd_parser_error(const cfd_parser_t *p);

#endif /* CFD_PARSER_H */
