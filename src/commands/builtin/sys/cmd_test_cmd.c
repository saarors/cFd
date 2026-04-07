#include "cmd_test_cmd.h"
#include "../../../utils/path.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/stat.h>
#  include <unistd.h>
#endif

static bool test_file_exists(const char *path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path, &st) == 0;
#endif
}

static bool test_is_dir(const char *path) {
    return cfd_path_is_dir(path);
}

static bool test_is_regular(const char *path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

static bool test_is_readable(const char *path) {
#ifdef _WIN32
    return test_file_exists(path); /* simplified */
#else
    return access(path, R_OK) == 0;
#endif
}

static bool test_is_writable(const char *path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_READONLY);
#else
    return access(path, W_OK) == 0;
#endif
}

static bool test_is_exec(const char *path) {
#ifdef _WIN32
    return test_file_exists(path);
#else
    return access(path, X_OK) == 0;
#endif
}

static bool is_num(const char *s) {
    if (!s || !*s) return false;
    int i = 0;
    if (s[0] == '-' || s[0] == '+') i = 1;
    if (!s[i]) return false;
    while (s[i]) {
        if (!isdigit((unsigned char)s[i])) return false;
        i++;
    }
    return true;
}

/* Evaluate the expression given by args[0..n-1] */
static bool test_eval(char **args, int n) {
    if (n == 0) return false;

    /* Parentheses */
    if (n >= 2 && strcmp(args[0], "(") == 0 && strcmp(args[n-1], ")") == 0)
        return test_eval(args + 1, n - 2);

    /* Negation */
    if (n >= 2 && strcmp(args[0], "!") == 0)
        return !test_eval(args + 1, n - 1);

    /* Compound: -a (and), -o (or) - scan right to left for lowest precedence */
    for (int i = n - 2; i >= 1; i--) {
        if (strcmp(args[i], "-o") == 0)
            return test_eval(args, i) || test_eval(args + i + 1, n - i - 1);
    }
    for (int i = n - 2; i >= 1; i--) {
        if (strcmp(args[i], "-a") == 0)
            return test_eval(args, i) && test_eval(args + i + 1, n - i - 1);
    }

    /* Unary */
    if (n == 2) {
        const char *op  = args[0];
        const char *arg = args[1];
        if (strcmp(op, "-f") == 0) return test_is_regular(arg);
        if (strcmp(op, "-d") == 0) return test_is_dir(arg);
        if (strcmp(op, "-e") == 0) return test_file_exists(arg);
        if (strcmp(op, "-r") == 0) return test_is_readable(arg);
        if (strcmp(op, "-w") == 0) return test_is_writable(arg);
        if (strcmp(op, "-x") == 0) return test_is_exec(arg);
        if (strcmp(op, "-z") == 0) return (strlen(arg) == 0);
        if (strcmp(op, "-n") == 0) return (strlen(arg) != 0);
        if (strcmp(op, "-s") == 0) {
            /* File exists and has size > 0 */
#ifdef _WIN32
            WIN32_FILE_ATTRIBUTE_DATA fad;
            if (!GetFileAttributesExA(arg, GetFileExInfoStandard, &fad)) return false;
            return (fad.nFileSizeLow > 0 || fad.nFileSizeHigh > 0);
#else
            struct stat st;
            return (stat(arg, &st) == 0 && st.st_size > 0);
#endif
        }
        if (strcmp(op, "!") == 0) {
            /* ! string -> string is empty */
            return (strlen(arg) == 0);
        }
    }

    /* Binary */
    if (n == 3) {
        const char *a  = args[0];
        const char *op = args[1];
        const char *b  = args[2];

        /* String comparisons */
        if (strcmp(op, "=")  == 0 || strcmp(op, "==") == 0) return strcmp(a, b) == 0;
        if (strcmp(op, "!=") == 0)                           return strcmp(a, b) != 0;
        if (strcmp(op, "<")  == 0)                           return strcmp(a, b) < 0;
        if (strcmp(op, ">")  == 0)                           return strcmp(a, b) > 0;

        /* Integer comparisons */
        if (is_num(a) && is_num(b)) {
            long la = atol(a), lb = atol(b);
            if (strcmp(op, "-eq") == 0) return la == lb;
            if (strcmp(op, "-ne") == 0) return la != lb;
            if (strcmp(op, "-lt") == 0) return la < lb;
            if (strcmp(op, "-gt") == 0) return la > lb;
            if (strcmp(op, "-le") == 0) return la <= lb;
            if (strcmp(op, "-ge") == 0) return la >= lb;
        }
    }

    /* Single argument: true if non-empty */
    if (n == 1) return (strlen(args[0]) != 0);

    return false;
}

int cmd_test_cmd(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    char **args = argv + 1;
    int    n    = argc - 1;

    /* Strip surrounding '[' ']' if called as [ ... ] */
    if (n > 0 && strcmp(args[0], "[") == 0) {
        args++;
        n--;
        if (n > 0 && strcmp(args[n-1], "]") == 0)
            n--;
    }

    return test_eval(args, n) ? 0 : 1;
}

const cfd_command_t builtin_test = {
    "test",
    "test <expr>",
    "Evaluate a conditional expression",
    "system",
    cmd_test_cmd,
    0, -1
};
