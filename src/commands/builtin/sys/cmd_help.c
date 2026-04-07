#include "cmd_help.h"
#include "../../../commands/registry.h"
#include "../../../ui/color.h"
#include "../../../ui/theme.h"
#include "../../../utils/mem.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void print_category(const char *cat, int *cmd_count, const char **names) {
    const cfd_theme_t *t = cfd_theme_get();
    printf("\n%s%s%s\n", cfd_color_fg(t->info_color), cat, cfd_color_reset());
    printf("  ");
    int col = 0;
    for (int i = 0; i < *cmd_count; i++) {
        const cfd_command_t *cmd = cfd_registry_find(g_registry, names[i]);
        if (!cmd || strcmp(cmd->category, cat) != 0) continue;
        printf("  %-12s", cmd->name);
        if (++col % 5 == 0) printf("\n  ");
    }
    printf("\n");
}

int cmd_help(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;
    const cfd_theme_t *t = cfd_theme_get();

    if (argc > 1) {
        const cfd_command_t *cmd = cfd_registry_find(g_registry, argv[1]);
        if (!cmd) { fprintf(stderr,"help: no such command: %s\n",argv[1]); return 1; }
        printf("%sNAME%s\n  %s\n\n%sUSAGE%s\n  %s\n\n%sDESCRIPTION%s\n  %s\n\n",
               cfd_color_fg(t->info_color), cfd_color_reset(), cmd->name,
               cfd_color_fg(t->info_color), cfd_color_reset(), cmd->usage,
               cfd_color_fg(t->info_color), cfd_color_reset(), cmd->description);
        return 0;
    }

    printf("%scFd built-in commands%s\n", cfd_color_fg(t->prompt_user), cfd_color_reset());
    printf("Type 'help <command>' for details on any built-in.\n\n");

    int n; const char **names = cfd_registry_list_names(&n);
    const char *cats[] = {
        "filesystem", "text", "system", "process",
        "network", "math", "crypto", "editor", "package",
        NULL
    };
    for (int c = 0; cats[c]; c++) {
        /* Only print category if it has members */
        bool has = false;
        for (int i = 0; i < n; i++) {
            const cfd_command_t *cmd = cfd_registry_find(g_registry, names[i]);
            if (cmd && strcmp(cmd->category, cats[c]) == 0) { has = true; break; }
        }
        if (has) print_category(cats[c], &n, names);
    }
    cfd_free((void*)names);

    printf("\n%sExternal programs%s\n", cfd_color_fg(t->info_color), cfd_color_reset());
    printf("  Any program in your PATH can be run directly:\n");
    printf("  e.g.  powershell   wsl   notepad   python   git   node\n");
    printf("  cFd searches PATH automatically — no need to type the full path.\n\n");
    return 0;
}
const cfd_command_t builtin_help = {
    "help","help [command]","Show help for commands","system",cmd_help,0,1
};
