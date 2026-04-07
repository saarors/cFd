#include "../include/version.h"
#include "core/terminal.h"
#include "utils/mem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_usage(const char *name) {
    fprintf(stderr,
        "Usage: %s [options] [script]\n"
        "\n"
        "Options:\n"
        "  -c <command>   Execute a command string and exit\n"
        "  -s             Read commands from stdin\n"
        "  --no-banner    Suppress startup banner\n"
        "  --no-color     Disable color output\n"
        "  --version      Print version and exit\n"
        "  --help         Print this help and exit\n"
        "\n"
        "If a script file is provided, it is executed non-interactively.\n",
        name);
}

int main(int argc, char *argv[]) {
    const char *cmd_str   = NULL;
    const char *script    = NULL;
    bool        no_banner = false;
    bool        no_color  = false;
    bool        from_stdin= false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("%s %s\n", CFD_NAME, CFD_VERSION);
            return 0;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 && i+1 < argc) {
            cmd_str = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            from_stdin = true;
        } else if (strcmp(argv[i], "--no-banner") == 0) {
            no_banner = true;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            no_color = true;
        } else if (argv[i][0] != '-') {
            script = argv[i];
        }
    }

    /* Create and init terminal */
    cfd_terminal_t *term = cfd_terminal_new();
    if (cfd_terminal_init(term) != 0) {
        fprintf(stderr, "cFd: failed to initialize\n");
        cfd_terminal_free(term);
        return 1;
    }

    if (no_banner) term->config->show_banner = false;
    if (no_color)  term->config->color_enabled = false;

    int ret = 0;

    if (cmd_str) {
        /* Execute single command */
        ret = cfd_session_exec_string(term->sess, cmd_str);
    } else if (script) {
        /* Run script file */
        ret = cfd_terminal_run_file(term, script);
    } else if (from_stdin) {
        /* Read from stdin */
        char line[4096];
        while (fgets(line, sizeof(line), stdin)) {
            size_t l = strlen(line);
            while (l > 0 && (line[l-1] == '\n' || line[l-1] == '\r')) line[--l] = '\0';
            if (*line && line[0] != '#')
                cfd_session_exec_string(term->sess, line);
        }
    } else {
        /* Interactive mode */
        ret = cfd_terminal_run(term);
    }

    cfd_terminal_shutdown(term);
    cfd_terminal_free(term);
    return ret;
}
