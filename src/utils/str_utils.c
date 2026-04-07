#include "str_utils.h"
#include "mem.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

char *cfd_strtrim_left(char *s) {
    if (!s) return s;
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

char *cfd_strtrim_right(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    return s;
}

char *cfd_strtrim(char *s) {
    return cfd_strtrim_right(cfd_strtrim_left(s));
}

char *cfd_strtolower(char *s) {
    if (!s) return s;
    for (char *p = s; *p; p++) *p = (char)tolower((unsigned char)*p);
    return s;
}

char *cfd_strtoupper(char *s) {
    if (!s) return s;
    for (char *p = s; *p; p++) *p = (char)toupper((unsigned char)*p);
    return s;
}

char **cfd_strsplit(const char *s, const char *delim, int *count) {
    if (!s || !delim) { if (count) *count = 0; return NULL; }
    int cap = 16, n = 0;
    char **parts = cfd_malloc(cap * sizeof(char *));
    char *copy = cfd_strdup(s);
    char *tok = strtok(copy, delim);
    while (tok) {
        if (n >= cap) {
            cap *= 2;
            parts = cfd_realloc(parts, cap * sizeof(char *));
        }
        parts[n++] = cfd_strdup(tok);
        tok = strtok(NULL, delim);
    }
    cfd_free(copy);
    parts = cfd_realloc(parts, (n + 1) * sizeof(char *));
    parts[n] = NULL;
    if (count) *count = n;
    return parts;
}

void cfd_strfreev(char **v) {
    if (!v) return;
    for (int i = 0; v[i]; i++) cfd_free(v[i]);
    cfd_free(v);
}

char *cfd_strjoin(char **parts, int count, const char *sep) {
    if (!parts || count == 0) return cfd_strdup("");
    size_t seplen = sep ? strlen(sep) : 0;
    size_t total = 0;
    for (int i = 0; i < count; i++) total += parts[i] ? strlen(parts[i]) : 0;
    total += seplen * (count > 1 ? count - 1 : 0) + 1;
    char *out = cfd_malloc(total);
    out[0] = '\0';
    for (int i = 0; i < count; i++) {
        if (i > 0 && sep) strcat(out, sep);
        if (parts[i]) strcat(out, parts[i]);
    }
    return out;
}

bool cfd_starts_with(const char *s, const char *prefix) {
    if (!s || !prefix) return false;
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

bool cfd_ends_with(const char *s, const char *suffix) {
    if (!s || !suffix) return false;
    size_t sl = strlen(s), pl = strlen(suffix);
    if (pl > sl) return false;
    return strcmp(s + sl - pl, suffix) == 0;
}

bool cfd_str_empty(const char *s) {
    return !s || *s == '\0';
}

char *cfd_strreplace(const char *src, const char *from, const char *to) {
    if (!src || !from || !to) return cfd_strdup(src ? src : "");
    size_t flen = strlen(from), tlen = strlen(to);
    size_t cap = strlen(src) * 2 + 64;
    char *out = cfd_malloc(cap);
    size_t oi = 0;
    const char *p = src;
    while (*p) {
        if (strncmp(p, from, flen) == 0) {
            if (oi + tlen + 1 >= cap) { cap = (oi + tlen) * 2 + 64; out = cfd_realloc(out, cap); }
            memcpy(out + oi, to, tlen); oi += tlen; p += flen;
        } else {
            if (oi + 2 >= cap) { cap *= 2; out = cfd_realloc(out, cap); }
            out[oi++] = *p++;
        }
    }
    out[oi] = '\0';
    return out;
}

char *cfd_sprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return cfd_strdup("");
    char *buf = cfd_malloc(n + 1);
    va_start(ap, fmt);
    vsnprintf(buf, n + 1, fmt, ap);
    va_end(ap);
    return buf;
}

int cfd_strcountc(const char *s, char c) {
    int n = 0;
    if (!s) return 0;
    while (*s) { if (*s++ == c) n++; }
    return n;
}

char *cfd_strrepeat(const char *s, int n) {
    if (!s || n <= 0) return cfd_strdup("");
    size_t len = strlen(s);
    char *out = cfd_malloc(len * n + 1);
    for (int i = 0; i < n; i++) memcpy(out + i * len, s, len);
    out[len * n] = '\0';
    return out;
}
