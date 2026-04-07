#include "platform.h"
#include "win32.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"

#ifdef _WIN32

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

static DWORD g_orig_in_mode  = 0;
static DWORD g_orig_out_mode = 0;
static HANDLE g_hin  = INVALID_HANDLE_VALUE;
static HANDLE g_hout = INVALID_HANDLE_VALUE;

/* Global child process handle for Ctrl+C forwarding */
static HANDLE g_child_process = INVALID_HANDLE_VALUE;

BOOL WINAPI cfd_ctrl_handler(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT) {
        if (g_child_process != INVALID_HANDLE_VALUE) {
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
            return TRUE;
        }
    }
    return FALSE;
}

void win32_enable_virtual_terminal(void) {
    g_hout = GetStdHandle(STD_OUTPUT_HANDLE);
    g_hin  = GetStdHandle(STD_INPUT_HANDLE);
    if (g_hout != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        GetConsoleMode(g_hout, &mode);
        g_orig_out_mode = mode;
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        SetConsoleMode(g_hout, mode);
    }
    if (g_hin != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        GetConsoleMode(g_hin, &mode);
        g_orig_in_mode = mode;
    }
    SetConsoleCtrlHandler(cfd_ctrl_handler, TRUE);
}

DWORD win32_get_console_mode(HANDLE h) {
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    return mode;
}

void win32_set_console_mode(HANDLE h, DWORD mode) {
    SetConsoleMode(h, mode);
}

const char *win32_strerror(DWORD err) {
    static char buf[512];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, buf, sizeof(buf), NULL);
    return buf;
}

int cfd_platform_raw_mode_enter(void) {
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return -1;
    DWORD mode;
    GetConsoleMode(h, &mode);
    /* Save current mode each time so we can restore correctly */
    g_orig_in_mode = mode;
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(h, mode);
    return 0;
}

int cfd_platform_raw_mode_exit(void) {
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return -1;
    SetConsoleMode(h, g_orig_in_mode);
    return 0;
}

int cfd_platform_get_terminal_size(int *cols, int *rows) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(h, &csbi)) {
        if (cols) *cols = 80;
        if (rows) *rows = 24;
        return -1;
    }
    if (cols) *cols = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
    if (rows) *rows = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
    return 0;
}

bool cfd_platform_color_supported(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(h, &mode)) return false;
    return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}

void cfd_platform_enable_ansi(void) {
    win32_enable_virtual_terminal();
    /* Also set code page to UTF-8 */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int cfd_platform_get_pid(void) {
    return (int)GetCurrentProcessId();
}

int cfd_platform_run_program(const char *path, char **argv, char **envp) {
    (void)envp;
    /* Build command line with proper quoting for args that contain spaces */
    char cmdline[8192] = {0};
    int pos = 0;
    for (int i = 0; argv && argv[i]; i++) {
        if (i > 0) { cmdline[pos++] = ' '; }
        const char *arg = argv[i];
        int needs_quote = 0;
        for (const char *c = arg; *c; c++) {
            if (*c == ' ' || *c == '\t' || *c == '"') { needs_quote = 1; break; }
        }
        if (needs_quote) {
            cmdline[pos++] = '"';
            for (const char *c = arg; *c; c++) {
                if (*c == '"') cmdline[pos++] = '\\';
                cmdline[pos++] = *c;
                if (pos >= (int)sizeof(cmdline) - 4) break;
            }
            cmdline[pos++] = '"';
        } else {
            int len = (int)strlen(arg);
            if (pos + len >= (int)sizeof(cmdline) - 2) break;
            memcpy(cmdline + pos, arg, len);
            pos += len;
        }
    }
    cmdline[pos] = '\0';

    /* Exit raw mode before running child so stdin is in cooked mode for child */
    cfd_platform_raw_mode_exit();

    STARTUPINFOA si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessA(path, cmdline, NULL, NULL, TRUE /*bInheritHandles*/,
                        0, NULL, NULL, &si, &pi)) {
        return -1;
    }

    /* Track child so Ctrl+C is forwarded */
    g_child_process = pi.hProcess;
    WaitForSingleObject(pi.hProcess, INFINITE);
    g_child_process = INVALID_HANDLE_VALUE;

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    /* Restore ANSI/VT mode in case the child changed it */
    cfd_platform_enable_ansi();

    return (int)code;
}

char *cfd_platform_getcwd(void) {
    char buf[4096];
    if (_getcwd(buf, sizeof(buf))) return cfd_strdup(buf);
    return cfd_strdup(".");
}

int cfd_platform_chdir(const char *path) {
    return _chdir(path);
}

char *cfd_platform_get_home(void) {
    const char *h = getenv("USERPROFILE");
    if (!h) h = getenv("HOMEDRIVE");
    if (!h) h = "C:\\";
    return cfd_strdup(h);
}

char *cfd_platform_getenv(const char *name) {
    const char *v = getenv(name);
    return v ? cfd_strdup(v) : NULL;
}

int cfd_platform_setenv(const char *name, const char *val) {
    return _putenv_s(name, val);
}

int cfd_platform_unsetenv(const char *name) {
    return _putenv_s(name, "");
}

/* Build a char** snapshot from the Windows environment block.
   We avoid _environ entirely because newer MinGW/UCRT builds export it
   as an import symbol (__imp__environ) that may be missing from the
   import library, causing a linker error. GetEnvironmentStrings() is
   the correct Win32 API and works on every Windows version. */
static char **g_env_cache = NULL;

char **cfd_platform_environ(void) {
    if (g_env_cache) return g_env_cache;

    LPCH block = GetEnvironmentStrings();
    if (!block) return NULL;

    /* count entries */
    int count = 0;
    for (LPCH p = block; *p; p += strlen(p) + 1) count++;

    g_env_cache = (char **)malloc((count + 1) * sizeof(char *));
    if (!g_env_cache) { FreeEnvironmentStrings(block); return NULL; }

    int i = 0;
    for (LPCH p = block; *p; p += strlen(p) + 1)
        g_env_cache[i++] = _strdup(p);
    g_env_cache[i] = NULL;

    FreeEnvironmentStrings(block);
    return g_env_cache;
}

int cfd_platform_read_key(void) {
    return _getch();
}

void cfd_platform_sleep_ms(unsigned ms) {
    Sleep(ms);
}

void cfd_platform_clear_screen(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count, written;
    COORD origin = {0, 0};
    if (!GetConsoleScreenBufferInfo(h, &csbi)) return;
    count = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacterA(h, ' ', count, origin, &written);
    FillConsoleOutputAttribute(h, csbi.wAttributes, count, origin, &written);
    SetConsoleCursorPosition(h, origin);
}

#endif /* _WIN32 */
