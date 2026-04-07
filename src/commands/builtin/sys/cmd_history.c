#include "cmd_history.h"
#include "../../../input/history.h"
#include "../../../input/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Access the session's input subsystem to get to history.
 * The session struct has an input pointer added by the shell main. */

/* We need to access the global input object. We rely on a weak reference
 * through session - if sess->input is available use it, otherwise skip. */

/* Since cfd_session_t does not have an 'input' field we use the scripting
 * variable store to find the history file path and load a fresh history view. */

int cmd_history(cfd_session_t *sess, int argc, char **argv) {
    /* Try to locate history via a well-known path */
    (void)sess;

    int  n       = -1;  /* -1 = all */
    bool clear   = false;
    int  del_idx = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            clear = true;
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            del_idx = atoi(argv[++i]);
        } else if (argv[i][0] != '-') {
            n = atoi(argv[i]);
        }
    }

    /* Build history file path */
#ifdef _WIN32
    const char *home = getenv("USERPROFILE");
#else
    const char *home = getenv("HOME");
#endif
    char histfile[512];
    if (home)
        snprintf(histfile, sizeof(histfile), "%s/.cfd_history", home);
    else
        strncpy(histfile, ".cfd_history", sizeof(histfile) - 1);

    if (clear) {
        FILE *fp = fopen(histfile, "w");
        if (fp) fclose(fp);
        return 0;
    }

    /* Load history file */
    FILE *fp = fopen(histfile, "r");
    if (!fp) {
        /* No history yet */
        return 0;
    }

    /* Read all lines */
    char **entries = NULL;
    int    count   = 0;
    int    cap     = 64;
    entries = (char **)malloc((size_t)cap * sizeof(char *));
    if (!entries) { fclose(fp); return 1; }

    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (count >= cap) {
            cap *= 2;
            char **tmp = (char **)realloc(entries, (size_t)cap * sizeof(char *));
            if (!tmp) break;
            entries = tmp;
        }
        entries[count++] = strdup(line);
    }
    fclose(fp);

    if (del_idx >= 1 && del_idx <= count) {
        /* Delete entry and rewrite file */
        free(entries[del_idx - 1]);
        memmove(entries + del_idx - 1, entries + del_idx,
                (size_t)(count - del_idx) * sizeof(char *));
        count--;

        fp = fopen(histfile, "w");
        if (fp) {
            for (int i = 0; i < count; i++) {
                fputs(entries[i], fp);
                fputc('\n', fp);
            }
            fclose(fp);
        }
        for (int i = 0; i < count; i++) free(entries[i]);
        free(entries);
        return 0;
    }

    /* Print last n entries */
    int start = 0;
    if (n > 0 && n < count) start = count - n;

    for (int i = start; i < count; i++)
        printf("  %4d  %s\n", i + 1, entries[i]);

    for (int i = 0; i < count; i++) free(entries[i]);
    free(entries);
    return 0;
}

const cfd_command_t builtin_history = {
    "history",
    "history [-c] [-d n] [n]",
    "Display or manipulate command history",
    "system",
    cmd_history,
    0, -1
};
