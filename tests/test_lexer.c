#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/parser/lexer.h"
#include "../src/parser/token.h"
#include "../src/utils/mem.h"

static int g_pass = 0, g_fail = 0;

static void check(const char *label, int cond) {
    if (cond) { g_pass++; printf("  PASS: %s\n", label); }
    else       { g_fail++; printf("  FAIL: %s\n", label); }
}

void run_lexer_tests(void) {
    /* Test 1: simple word */
    {
        cfd_list_t *toks = cfd_lexer_tokenize("hello");
        cfd_token_t *t = cfd_list_get(toks, 0);
        check("simple word type", t->type == TOK_WORD);
        check("simple word value", t->value && strcmp(t->value,"hello")==0);
        cfd_list_free(toks);
    }
    /* Test 2: pipe */
    {
        cfd_list_t *toks = cfd_lexer_tokenize("a | b");
        cfd_token_t *t0=cfd_list_get(toks,0);
        cfd_token_t *t1=cfd_list_get(toks,1);
        cfd_token_t *t2=cfd_list_get(toks,2);
        check("pipe: left word",  t0->type==TOK_WORD);
        check("pipe: pipe token", t1->type==TOK_PIPE);
        check("pipe: right word", t2->type==TOK_WORD);
        cfd_list_free(toks);
    }
    /* Test 3: quoted string */
    {
        cfd_list_t *toks = cfd_lexer_tokenize("\"hello world\"");
        cfd_token_t *t = cfd_list_get(toks,0);
        check("quoted string type",  t->type==TOK_STRING);
        check("quoted string value", t->value && strcmp(t->value,"hello world")==0);
        cfd_list_free(toks);
    }
    /* Test 4: && and || */
    {
        cfd_list_t *toks = cfd_lexer_tokenize("a && b || c");
        cfd_token_t *and_tok = cfd_list_get(toks,1);
        cfd_token_t *or_tok  = cfd_list_get(toks,3);
        check("AND token", and_tok->type==TOK_AND);
        check("OR token",  or_tok->type==TOK_OR);
        cfd_list_free(toks);
    }
    /* Test 5: redirects */
    {
        cfd_list_t *toks = cfd_lexer_tokenize("cmd > out.txt");
        cfd_token_t *r = cfd_list_get(toks,1);
        check("redir out", r->type==TOK_REDIR_OUT);
        cfd_list_free(toks);
    }
}
