#include "io.h"
#include "../utils/mem.h"
#include <stdio.h>
#include <string.h>

cfd_io_ctx_t *cfd_io_ctx_new(void) {
    cfd_io_ctx_t *ctx = CFD_NEW(cfd_io_ctx_t);
    ctx->stdin_stream  = cfd_stream_from_file(stdin,  false);
    ctx->stdout_stream = cfd_stream_from_file(stdout, false);
    ctx->stderr_stream = cfd_stream_from_file(stderr, false);
    return ctx;
}

void cfd_io_ctx_free(cfd_io_ctx_t *ctx) {
    if (!ctx) return;
    cfd_stream_free(ctx->stdin_stream);
    cfd_stream_free(ctx->stdout_stream);
    cfd_stream_free(ctx->stderr_stream);
    cfd_free(ctx);
}

void cfd_io_ctx_reset(cfd_io_ctx_t *ctx) {
    if (!ctx) return;
    cfd_stream_free(ctx->stdin_stream);
    cfd_stream_free(ctx->stdout_stream);
    cfd_stream_free(ctx->stderr_stream);
    ctx->stdin_stream  = cfd_stream_from_file(stdin,  false);
    ctx->stdout_stream = cfd_stream_from_file(stdout, false);
    ctx->stderr_stream = cfd_stream_from_file(stderr, false);
}

char *cfd_io_read_all(cfd_stream_t *s) {
    if (!s) return cfd_strdup("");
    char  *buf = cfd_malloc(4096);
    size_t cap = 4096, len = 0;
    int c;
    while ((c = cfd_stream_getc(s)) != EOF) {
        if (len + 2 >= cap) { cap *= 2; buf = cfd_realloc(buf, cap); }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}

int cfd_io_copy(cfd_stream_t *src, cfd_stream_t *dst) {
    if (!src || !dst) return -1;
    int c, n = 0;
    while ((c = cfd_stream_getc(src)) != EOF) {
        cfd_stream_putc(dst, c);
        n++;
    }
    return n;
}
