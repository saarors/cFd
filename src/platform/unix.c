#include "platform.h"
#include "unix.h"
#include "../utils/mem.h"

#ifndef _WIN32

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct termios g_orig_termios;
static int g_raw_mode = 0;

void unix_restore_terminal(void) {
    if (g_raw_mode) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
        g_raw_mode = 0;
    }
}

int cfd_platform_raw_mode_enter(void) {
    if (!isatty(STDIN_FILENO)) return -1;
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) != 0) return -1;
    struct termios raw = g_orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_cflag |=  CS8;
    raw.c_oflag &= ~OPOST;
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) return -1;
    g_raw_mode = 1;
    return 0;
}

int cfd_platform_raw_mode_exit(void) {
    unix_restore_terminal();
    return 0;
}

int cfd_platform_get_terminal_size(int *cols, int *rows) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        if (cols) *cols = 80;
        if (rows) *rows = 24;
        return -1;
    }
    if (cols) *cols = ws.ws_col;
    if (rows) *rows = ws.ws_row;
    return 0;
}

bool cfd_platform_color_supported(void) {
    const char *term = getenv("TERM");
    if (!term) return false;
    return strstr(term, "color") || strstr(term, "xterm") ||
           strstr(term, "screen") || strstr(term, "256");
}

void cfd_platform_enable_ansi(void) { /* no-op on Unix */ }

int cfd_platform_get_pid(void) {
    return (int)getpid();
}

int cfd_platform_run_program(const char *path, char **argv, char **envp) {
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        if (envp) execve(path, argv, envp);
        else      execv(path, argv);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

char *cfd_platform_getcwd(void) {
    char buf[4096];
    if (getcwd(buf, sizeof(buf))) return cfd_strdup(buf);
    return cfd_strdup(".");
}

int cfd_platform_chdir(const char *path) {
    return chdir(path);
}

char *cfd_platform_get_home(void) {
    const char *h = getenv("HOME");
    return cfd_strdup(h ? h : "/");
}

char *cfd_platform_getenv(const char *name) {
    const char *v = getenv(name);
    return v ? cfd_strdup(v) : NULL;
}

extern char **environ;

int cfd_platform_setenv(const char *name, const char *val) {
    return setenv(name, val, 1);
}

int cfd_platform_unsetenv(const char *name) {
    return unsetenv(name);
}

char **cfd_platform_environ(void) {
    return environ;
}

int cfd_platform_read_key(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1) return (int)c;
    return -1;
}

void cfd_platform_sleep_ms(unsigned ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void cfd_platform_clear_screen(void) {
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
}

#endif /* _WIN32 */
