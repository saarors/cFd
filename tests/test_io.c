#include <stdio.h>
#include <string.h>
#include "../src/io/stream.h"
#include "../src/utils/mem.h"

static int g_pass = 0, g_fail = 0;

static void check(const char *label, int cond) {
    if (cond) { g_pass++; printf("  PASS: %s\n", label); }
    else       { g_fail++; printf("  FAIL: %s\n", label); }
}

void run_io_tests(void) {
    /* String stream read */
    {
        cfd_stream_t *s = cfd_stream_from_string("hello\nworld\n");
        char *line1 = cfd_stream_getline(s);
        char *line2 = cfd_stream_getline(s);
        char *line3 = cfd_stream_getline(s);
        check("stream line1", line1 && strcmp(line1,"hello")==0);
        check("stream line2", line2 && strcmp(line2,"world")==0);
        check("stream eof",   line3 == NULL);
        cfd_free(line1); cfd_free(line2);
        cfd_stream_free(s);
    }
    /* String stream write */
    {
        cfd_stream_t *s = cfd_stream_new_string_buf();
        cfd_stream_puts(s, "hello");
        cfd_stream_puts(s, " world");
        const char *buf = cfd_stream_buf_contents(s);
        check("stream write buf", buf && strcmp(buf,"hello world")==0);
        cfd_stream_free(s);
    }
    /* Getc / putc */
    {
        cfd_stream_t *s = cfd_stream_from_string("AB");
        check("getc A", cfd_stream_getc(s)=='A');
        check("getc B", cfd_stream_getc(s)=='B');
        check("getc EOF", cfd_stream_getc(s)==EOF);
        cfd_stream_free(s);
    }
}
