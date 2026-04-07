#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/parser/parser.h"
#include "../src/parser/ast.h"

static int g_pass = 0, g_fail = 0;

static void check(const char *label, int cond) {
    if (cond) { g_pass++; printf("  PASS: %s\n", label); }
    else       { g_fail++; printf("  FAIL: %s\n", label); }
}

void run_parser_tests(void) {
    /* Test 1: simple command */
    {
        cfd_ast_node_t *n = cfd_parse("echo hello");
        check("simple cmd type",  n && n->type == AST_CMD);
        check("simple cmd argc",  n && n->argc == 2);
        check("simple cmd argv0", n && strcmp(n->argv[0],"echo")==0);
        check("simple cmd argv1", n && strcmp(n->argv[1],"hello")==0);
        cfd_ast_free(n);
    }
    /* Test 2: pipeline */
    {
        cfd_ast_node_t *n = cfd_parse("cat file | grep word");
        check("pipeline type", n && n->type == AST_PIPELINE);
        check("pipeline len",  n && n->pipeline_len == 2);
        cfd_ast_free(n);
    }
    /* Test 3: list with semicolon */
    {
        cfd_ast_node_t *n = cfd_parse("echo a ; echo b");
        check("list type", n && n->type == AST_LIST);
        cfd_ast_free(n);
    }
    /* Test 4: AND */
    {
        cfd_ast_node_t *n = cfd_parse("mkdir dir && cd dir");
        check("and type", n && n->type == AST_AND);
        cfd_ast_free(n);
    }
    /* Test 5: assignment */
    {
        cfd_ast_node_t *n = cfd_parse("FOO=bar");
        check("assign type",  n && n->type == AST_ASSIGN);
        check("assign key",   n && strcmp(n->assign_key,"FOO")==0);
        check("assign val",   n && strcmp(n->assign_val,"bar")==0);
        cfd_ast_free(n);
    }
}
