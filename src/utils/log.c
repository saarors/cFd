#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

static FILE          *g_logfile   = NULL;
static cfd_log_level_t g_min_level = CFD_LOG_WARN;

static const char *level_str(cfd_log_level_t level) {
    switch (level) {
        case CFD_LOG_DEBUG: return "DEBUG";
        case CFD_LOG_INFO:  return "INFO";
        case CFD_LOG_WARN:  return "WARN";
        case CFD_LOG_ERROR: return "ERROR";
        case CFD_LOG_FATAL: return "FATAL";
        default:            return "?";
    }
}

void cfd_log_init(const char *logfile, cfd_log_level_t min_level) {
    g_min_level = min_level;
    if (logfile) {
        g_logfile = fopen(logfile, "a");
    }
}

void cfd_log_close(void) {
    if (g_logfile) { fclose(g_logfile); g_logfile = NULL; }
}

void cfd_log_set_level(cfd_log_level_t level) {
    g_min_level = level;
}

void cfd_log(cfd_log_level_t level, const char *fmt, ...) {
    if (level < g_min_level) return;
    FILE *out = g_logfile ? g_logfile : stderr;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    fprintf(out, "[%s] [%s] ", ts, level_str(level));
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fprintf(out, "\n");
    fflush(out);
}
