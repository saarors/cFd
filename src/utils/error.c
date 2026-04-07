#include "error.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static cfd_err_t  g_error_code = 0;
static char       g_error_msg[CFD_MAX_ERROR] = {0};

void cfd_set_error(cfd_err_t code, const char *fmt, ...) {
    g_error_code = code;
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(g_error_msg, sizeof(g_error_msg), fmt, ap);
        va_end(ap);
    } else {
        g_error_msg[0] = '\0';
    }
}

cfd_err_t cfd_get_error_code(void) {
    return g_error_code;
}

const char *cfd_get_error_msg(void) {
    return g_error_msg;
}

void cfd_clear_error(void) {
    g_error_code = 0;
    g_error_msg[0] = '\0';
}

void cfd_perror(const char *prefix) {
    if (prefix && *prefix)
        fprintf(stderr, "%s: %s\n", prefix, g_error_msg);
    else
        fprintf(stderr, "%s\n", g_error_msg);
}

void cfd_panic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "cFd: PANIC: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}
