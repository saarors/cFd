#ifndef CFD_PIPE_H
#define CFD_PIPE_H

#include <stdio.h>
#include <stdbool.h>

typedef struct cfd_pipe {
    int read_fd;
    int write_fd;
    FILE *read_fp;
    FILE *write_fp;
} cfd_pipe_t;

int         cfd_pipe_create(cfd_pipe_t *p);
void        cfd_pipe_close_read(cfd_pipe_t *p);
void        cfd_pipe_close_write(cfd_pipe_t *p);
void        cfd_pipe_close(cfd_pipe_t *p);

FILE       *cfd_pipe_read_stream(cfd_pipe_t *p);
FILE       *cfd_pipe_write_stream(cfd_pipe_t *p);

#endif /* CFD_PIPE_H */
