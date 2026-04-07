#ifndef CFD_STR_UTILS_H
#define CFD_STR_UTILS_H

#include <stddef.h>
#include <stdbool.h>

/* Trim whitespace */
char *cfd_strtrim(char *s);
char *cfd_strtrim_left(char *s);
char *cfd_strtrim_right(char *s);

/* Case conversion */
char *cfd_strtolower(char *s);
char *cfd_strtoupper(char *s);

/* Splitting */
char **cfd_strsplit(const char *s, const char *delim, int *count);
void   cfd_strfreev(char **v);

/* Joining */
char *cfd_strjoin(char **parts, int count, const char *sep);

/* Checks */
bool cfd_starts_with(const char *s, const char *prefix);
bool cfd_ends_with(const char *s, const char *suffix);
bool cfd_str_empty(const char *s);

/* Replacement */
char *cfd_strreplace(const char *src, const char *from, const char *to);

/* Safe sprintf into dynamic buffer */
char *cfd_sprintf(const char *fmt, ...);

/* Count occurrences */
int cfd_strcountc(const char *s, char c);

/* Repeat */
char *cfd_strrepeat(const char *s, int n);

#endif /* CFD_STR_UTILS_H */
