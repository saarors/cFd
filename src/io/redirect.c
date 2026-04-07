#include "redirect.h"
#include "../parser/token.h"
#include "../utils/mem.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <io.h>
#  define dup(fd)        _dup(fd)
#  define dup2(fd1,fd2)  _dup2(fd1,fd2)
#  define close(fd)      _close(fd)
#  define open(p,f,m)    _open(p,f,m)
#  define O_RDONLY _O_RDONLY
#  define O_WRONLY _O_WRONLY
#  define O_CREAT  _O_CREAT
#  define O_TRUNC  _O_TRUNC
#  define O_APPEND _O_APPEND
#  ifndef S_IRUSR
#    define S_IRUSR  _S_IREAD
#    define S_IWUSR  _S_IWRITE
#  endif
#  ifndef O_BINARY
#    define O_BINARY _O_BINARY
#  endif
#else
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  define O_BINARY 0
#endif

static void save_fd(cfd_redirect_ctx_t *ctx, int fd) {
    if (ctx->count >= 16) return;
    ctx->saved[ctx->count].orig_fd  = fd;
    ctx->saved[ctx->count].saved_fd = dup(fd);
    ctx->count++;
}

int cfd_redirect_apply(cfd_redirect_node_t *rd, cfd_redirect_ctx_t *ctx) {
    ctx->count = 0;
    for (; rd; rd = rd->next) {
        switch (rd->type) {
            case TOK_REDIR_IN: {
                int fd = open(rd->target, O_RDONLY | O_BINARY, 0);
                if (fd < 0) { perror(rd->target); return -1; }
                save_fd(ctx, 0);
                dup2(fd, 0);
                close(fd);
                break;
            }
            case TOK_REDIR_OUT: {
                int fd = open(rd->target, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
                              S_IRUSR | S_IWUSR);
                if (fd < 0) { perror(rd->target); return -1; }
                save_fd(ctx, 1);
                dup2(fd, 1);
                close(fd);
                break;
            }
            case TOK_REDIR_APPEND: {
                int fd = open(rd->target, O_WRONLY | O_CREAT | O_APPEND | O_BINARY,
                              S_IRUSR | S_IWUSR);
                if (fd < 0) { perror(rd->target); return -1; }
                save_fd(ctx, 1);
                dup2(fd, 1);
                close(fd);
                break;
            }
            case TOK_REDIR_ERR: {
                int fd = open(rd->target, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
                              S_IRUSR | S_IWUSR);
                if (fd < 0) { perror(rd->target); return -1; }
                save_fd(ctx, 2);
                dup2(fd, 2);
                close(fd);
                break;
            }
            case TOK_REDIR_ERR_OUT: {
                save_fd(ctx, 2);
                dup2(1, 2);
                break;
            }
            default: break;
        }
    }
    return 0;
}

void cfd_redirect_restore(cfd_redirect_ctx_t *ctx) {
    for (int i = ctx->count - 1; i >= 0; i--) {
        dup2(ctx->saved[i].saved_fd, ctx->saved[i].orig_fd);
        close(ctx->saved[i].saved_fd);
    }
    ctx->count = 0;
}
