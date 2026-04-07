#ifndef CFD_STREAM_H
#define CFD_STREAM_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    STREAM_FILE   = 0,
    STREAM_PIPE   = 1,
    STREAM_STRING = 2
} cfd_stream_kind_t;

typedef struct cfd_stream {
    cfd_stream_kind_t kind;
    FILE             *fp;
    char             *buf;
    size_t            buf_cap;
    size_t            buf_len;
    size_t            buf_pos;
    bool              owner;    /* we own fp or buf */
} cfd_stream_t;

cfd_stream_t *cfd_stream_from_file(FILE *fp, bool owner);
cfd_stream_t *cfd_stream_from_path(const char *path, const char *mode);
cfd_stream_t *cfd_stream_from_string(const char *s);
cfd_stream_t *cfd_stream_new_string_buf(void);

void  cfd_stream_free(cfd_stream_t *s);

int   cfd_stream_getc(cfd_stream_t *s);
char *cfd_stream_getline(cfd_stream_t *s);
int   cfd_stream_putc(cfd_stream_t *s, int c);
int   cfd_stream_puts(cfd_stream_t *s, const char *str);
int   cfd_stream_printf(cfd_stream_t *s, const char *fmt, ...);

bool  cfd_stream_eof(cfd_stream_t *s);
char *cfd_stream_buf_contents(cfd_stream_t *s);

#endif /* CFD_STREAM_H */
