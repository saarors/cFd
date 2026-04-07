#ifndef CFD_PATH_H
#define CFD_PATH_H

#include <stdbool.h>
#include <stddef.h>

char *cfd_path_join(const char *a, const char *b);
char *cfd_path_dirname(const char *path);
char *cfd_path_basename(const char *path);
char *cfd_path_ext(const char *path);
char *cfd_path_normalize(const char *path);
char *cfd_path_expand_home(const char *path);
char *cfd_path_absolute(const char *path);

bool  cfd_path_exists(const char *path);
bool  cfd_path_is_dir(const char *path);
bool  cfd_path_is_file(const char *path);
bool  cfd_path_is_absolute(const char *path);

char **cfd_path_split_dirs(const char *env_path, int *count);

#endif /* CFD_PATH_H */
