#ifndef CFD_PLATFORM_H
#define CFD_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>

/* Detect platform */
#ifdef _WIN32
#  define CFD_PLATFORM_WINDOWS 1
#  define CFD_PLATFORM_NAME    "Windows"
#else
#  define CFD_PLATFORM_UNIX    1
#  define CFD_PLATFORM_NAME    "Unix"
#endif

/* Terminal raw mode */
int  cfd_platform_raw_mode_enter(void);
int  cfd_platform_raw_mode_exit(void);

/* Terminal size */
int  cfd_platform_get_terminal_size(int *cols, int *rows);

/* Console color support */
bool cfd_platform_color_supported(void);
void cfd_platform_enable_ansi(void);

/* Process */
int  cfd_platform_get_pid(void);
int  cfd_platform_run_program(const char *path, char **argv, char **envp);

/* Working directory */
char *cfd_platform_getcwd(void);
int   cfd_platform_chdir(const char *path);

/* Home directory */
char *cfd_platform_get_home(void);

/* Environment */
char *cfd_platform_getenv(const char *name);
int   cfd_platform_setenv(const char *name, const char *val);
int   cfd_platform_unsetenv(const char *name);
char **cfd_platform_environ(void);

/* Key reading (single character, no echo) */
int cfd_platform_read_key(void);

/* Sleep milliseconds */
void cfd_platform_sleep_ms(unsigned ms);

/* Clear screen */
void cfd_platform_clear_screen(void);

#endif /* CFD_PLATFORM_H */
