#ifndef CFD_IO_H
#define CFD_IO_H

#include "stream.h"
#include "pipe.h"
#include "redirect.h"
#include "../../include/types.h"

typedef struct cfd_io_ctx {
    cfd_stream_t *stdin_stream;
    cfd_stream_t *stdout_stream;
    cfd_stream_t *stderr_stream;
} cfd_io_ctx_t;

cfd_io_ctx_t *cfd_io_ctx_new(void);
void          cfd_io_ctx_free(cfd_io_ctx_t *ctx);
void          cfd_io_ctx_reset(cfd_io_ctx_t *ctx);

/* Read all from a stream into a dynamically allocated string */
char *cfd_io_read_all(cfd_stream_t *s);

/* Copy data between streams */
int   cfd_io_copy(cfd_stream_t *src, cfd_stream_t *dst);

#endif /* CFD_IO_H */
