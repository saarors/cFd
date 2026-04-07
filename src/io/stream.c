#include "stream.h"
#include "../utils/mem.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

cfd_stream_t *cfd_stream_from_file(FILE *fp, bool owner) {
    cfd_stream_t *s = CFD_NEW(cfd_stream_t);
    s->kind  = STREAM_FILE;
    s->fp    = fp;
    s->owner = owner;
    return s;
}

cfd_stream_t *cfd_stream_from_path(const char *path, const char *mode) {
    FILE *fp = fopen(path, mode);
    if (!fp) return NULL;
    return cfd_stream_from_file(fp, true);
}

cfd_stream_t *cfd_stream_from_string(const char *s) {
    cfd_stream_t *st = CFD_NEW(cfd_stream_t);
    st->kind    = STREAM_STRING;
    st->buf     = cfd_strdup(s ? s : "");
    st->buf_len = strlen(st->buf);
    st->buf_cap = st->buf_len + 1;
    st->owner   = true;
    return st;
}

cfd_stream_t *cfd_stream_new_string_buf(void) {
    cfd_stream_t *st = CFD_NEW(cfd_stream_t);
    st->kind    = STREAM_STRING;
    st->buf_cap = 256;
    st->buf     = cfd_calloc(st->buf_cap, 1);
    st->owner   = true;
    return st;
}

void cfd_stream_free(cfd_stream_t *s) {
    if (!s) return;
    if (s->owner) {
        if (s->fp) fclose(s->fp);
        cfd_free(s->buf);
    }
    cfd_free(s);
}

int cfd_stream_getc(cfd_stream_t *s) {
    if (!s) return EOF;
    if (s->kind == STREAM_FILE) return fgetc(s->fp);
    if (s->buf_pos >= s->buf_len) return EOF;
    return (unsigned char)s->buf[s->buf_pos++];
}

char *cfd_stream_getline(cfd_stream_t *s) {
    if (!s) return NULL;
    char buf[4096];
    size_t n = 0;
    int c;
    while ((c = cfd_stream_getc(s)) != EOF && c != '\n') {
        if (n < sizeof(buf) - 1) buf[n++] = (char)c;
    }
    if (c == EOF && n == 0) return NULL;
    buf[n] = '\0';
    return cfd_strdup(buf);
}

int cfd_stream_putc(cfd_stream_t *s, int c) {
    if (!s) return -1;
    if (s->kind == STREAM_FILE) return fputc(c, s->fp);
    if (s->buf_len + 1 >= s->buf_cap) {
        s->buf_cap *= 2;
        s->buf = cfd_realloc(s->buf, s->buf_cap);
    }
    s->buf[s->buf_len++] = (char)c;
    s->buf[s->buf_len]   = '\0';
    return c;
}

int cfd_stream_puts(cfd_stream_t *s, const char *str) {
    if (!s || !str) return -1;
    if (s->kind == STREAM_FILE) return fputs(str, s->fp);
    size_t len = strlen(str);
    while (s->buf_len + len + 1 >= s->buf_cap) {
        s->buf_cap *= 2;
        if (s->buf_cap < 16) s->buf_cap = 16;
        s->buf = cfd_realloc(s->buf, s->buf_cap);
    }
    memcpy(s->buf + s->buf_len, str, len + 1);
    s->buf_len += len;
    return (int)len;
}

int cfd_stream_printf(cfd_stream_t *s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    if (!s) { va_end(ap); return -1; }
    if (s->kind == STREAM_FILE) {
        int r = vfprintf(s->fp, fmt, ap);
        va_end(ap);
        return r;
    }
    char tmp[4096];
    int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    cfd_stream_puts(s, tmp);
    return n;
}

bool cfd_stream_eof(cfd_stream_t *s) {
    if (!s) return true;
    if (s->kind == STREAM_FILE) return feof(s->fp);
    return s->buf_pos >= s->buf_len;
}

char *cfd_stream_buf_contents(cfd_stream_t *s) {
    if (!s || s->kind != STREAM_STRING) return NULL;
    return s->buf;
}
