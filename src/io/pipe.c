#include "pipe.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#  include <io.h>
#  include <fcntl.h>
#  define pipe(fds) _pipe(fds, 4096, O_BINARY)
#else
#  include <unistd.h>
#endif

int cfd_pipe_create(cfd_pipe_t *p) {
    int fds[2];
    if (pipe(fds) != 0) return -1;
    p->read_fd  = fds[0];
    p->write_fd = fds[1];
    p->read_fp  = NULL;
    p->write_fp = NULL;
    return 0;
}

void cfd_pipe_close_read(cfd_pipe_t *p) {
    if (p->read_fp)  { fclose(p->read_fp);  p->read_fp  = NULL; }
    else if (p->read_fd  >= 0) {
#ifdef _WIN32
        _close(p->read_fd);
#else
        close(p->read_fd);
#endif
        p->read_fd = -1;
    }
}

void cfd_pipe_close_write(cfd_pipe_t *p) {
    if (p->write_fp) { fclose(p->write_fp); p->write_fp = NULL; }
    else if (p->write_fd >= 0) {
#ifdef _WIN32
        _close(p->write_fd);
#else
        close(p->write_fd);
#endif
        p->write_fd = -1;
    }
}

void cfd_pipe_close(cfd_pipe_t *p) {
    cfd_pipe_close_read(p);
    cfd_pipe_close_write(p);
}

FILE *cfd_pipe_read_stream(cfd_pipe_t *p) {
    if (!p->read_fp && p->read_fd >= 0)
#ifdef _WIN32
        p->read_fp = _fdopen(p->read_fd, "r");
#else
        p->read_fp = fdopen(p->read_fd, "r");
#endif
    return p->read_fp;
}

FILE *cfd_pipe_write_stream(cfd_pipe_t *p) {
    if (!p->write_fp && p->write_fd >= 0)
#ifdef _WIN32
        p->write_fp = _fdopen(p->write_fd, "w");
#else
        p->write_fp = fdopen(p->write_fd, "w");
#endif
    return p->write_fp;
}
