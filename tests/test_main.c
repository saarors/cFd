#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Simple test framework */
static int g_pass = 0, g_fail = 0;
#define TEST(name) static void test_##name(void)
#define RUN(name)  do { printf("  %-40s", #name); test_##name(); } while(0)
#define ASSERT(cond) do { \
    if(cond){g_pass++;printf("PASS\n");}else{g_fail++;printf("FAIL (line %d)\n",__LINE__);} \
} while(0)

/* Import tests from other files */
extern void run_lexer_tests(void);
extern void run_parser_tests(void);
extern void run_utils_tests(void);
extern void run_io_tests(void);

int main(void) {
    printf("\n=== cFd Test Suite ===\n\n");

    printf("[Lexer Tests]\n");
    run_lexer_tests();

    printf("\n[Parser Tests]\n");
    run_parser_tests();

    printf("\n[Utils Tests]\n");
    run_utils_tests();

    printf("\n[I/O Tests]\n");
    run_io_tests();

    printf("\n=== Results: %d passed, %d failed ===\n\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
