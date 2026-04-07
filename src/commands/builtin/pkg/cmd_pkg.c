#include "cmd_pkg.h"
#include "../../../utils/mem.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ---- Built-in package registry ---- */
static struct {
    const char *name;
    const char *winget_id;
    const char *description;
} pkg_registry[] = {
    {"git",        "Git.Git",                       "Distributed version control"},
    {"nodejs",     "OpenJS.NodeJS",                 "JavaScript runtime"},
    {"python",     "Python.Python.3",               "Python 3 interpreter"},
    {"vscode",     "Microsoft.VisualStudioCode",    "Code editor"},
    {"7zip",       "7zip.7zip",                     "File archiver"},
    {"vlc",        "VideoLAN.VLC",                  "Media player"},
    {"firefox",    "Mozilla.Firefox",               "Web browser"},
    {"chrome",     "Google.Chrome",                 "Web browser"},
    {"notepad++",  "Notepad++.Notepad++",           "Text editor"},
    {"cmake",      "Kitware.CMake",                 "Build system"},
    {"make",       "GnuWin32.Make",                 "Build tool"},
    {"gcc",        "MSYS2.MSYS2",                   "GNU compiler (via MSYS2)"},
    {"rust",       "Rustlang.Rustup",               "Rust toolchain"},
    {"go",         "GoLang.Go",                     "Go programming language"},
    {"java",       "Oracle.JDK.21",                 "Java Development Kit"},
    {"docker",     "Docker.DockerDesktop",          "Container platform"},
    {"kubectl",    "Kubernetes.kubectl",             "Kubernetes CLI"},
    {"terraform",  "Hashicorp.Terraform",           "Infrastructure as code"},
    {"neovim",     "Neovim.Neovim",                 "Modern vim"},
    {"htop",       "htop-dev.htop",                 "Process viewer"},
    {"ffmpeg",     "Gyan.FFmpeg",                   "Media converter"},
    {"curl",       "cURL.cURL",                     "HTTP client"},
    {"wget",       "GnuWin32.Wget",                 "File downloader"},
    {"ripgrep",    "BurntSushi.ripgrep",            "Fast grep"},
    {"fzf",        "junegunn.fzf",                  "Fuzzy finder"},
    {"bat",        "sharkdp.bat",                   "Better cat"},
    {"fd",         "sharkdp.fd",                    "Better find"},
    {"jq",         "jqlang.jq",                     "JSON processor"},
    {"gh",         "GitHub.cli",                    "GitHub CLI"},
    {"tmux",       "tmux.tmux",                     "Terminal multiplexer"},
    {"vim",        "vim.vim",                       "Text editor"},
    {NULL, NULL, NULL}
};

/* ---- Color helpers ---- */
#define PKG_COLOR_GREEN  "\033[32m"
#define PKG_COLOR_RED    "\033[31m"
#define PKG_COLOR_YELLOW "\033[33m"
#define PKG_COLOR_CYAN   "\033[36m"
#define PKG_COLOR_RESET  "\033[0m"

static void pkg_ok(const char *msg) {
    printf("%s%s%s\n", PKG_COLOR_GREEN, msg, PKG_COLOR_RESET);
}
static void pkg_err(const char *msg) {
    fprintf(stderr, "%s%s%s\n", PKG_COLOR_RED, msg, PKG_COLOR_RESET);
}
static void pkg_info(const char *msg) {
    printf("%s%s%s\n", PKG_COLOR_YELLOW, msg, PKG_COLOR_RESET);
}

/* ---- Package file path ---- */
static void get_pkg_file(char *buf, size_t sz) {
    char *home = cfd_platform_get_home();
    if (home) {
        snprintf(buf, sz, "%s/.cfd_packages", home);
        cfd_free(home);
    } else {
        strncpy(buf, ".cfd_packages", sz - 1);
        buf[sz - 1] = '\0';
    }
}

static bool pkg_is_installed(const char *name) {
    char path[512];
    get_pkg_file(path, sizeof(path));
    FILE *fp = fopen(path, "r");
    if (!fp) return false;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (strcmp(line, name) == 0) { fclose(fp); return true; }
    }
    fclose(fp);
    return false;
}

static void pkg_mark_installed(const char *name) {
    char path[512];
    get_pkg_file(path, sizeof(path));
    FILE *fp = fopen(path, "a");
    if (!fp) return;
    fprintf(fp, "%s\n", name);
    fclose(fp);
}

static void pkg_unmark_installed(const char *name) {
    char path[512];
    get_pkg_file(path, sizeof(path));

    char tmp_path[520];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    FILE *fp  = fopen(path, "r");
    FILE *out = fopen(tmp_path, "w");
    if (!fp || !out) {
        if (fp)  fclose(fp);
        if (out) fclose(out);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (strcmp(line, name) != 0)
            fprintf(out, "%s\n", line);
    }
    fclose(fp);
    fclose(out);
    remove(path);
    rename(tmp_path, path);
}

/* ---- Detect package manager on Windows ---- */
#ifdef _WIN32
typedef enum { PM_WINGET, PM_SCOOP, PM_CHOCO, PM_NONE } pkg_manager_t;

static pkg_manager_t detect_win_pm(void) {
    if (system("where winget >nul 2>&1") == 0) return PM_WINGET;
    if (system("where scoop >nul 2>&1")  == 0) return PM_SCOOP;
    if (system("where choco >nul 2>&1")  == 0) return PM_CHOCO;
    return PM_NONE;
}

static const char *find_winget_id(const char *name) {
    for (int i = 0; pkg_registry[i].name; i++) {
        if (strcmp(pkg_registry[i].name, name) == 0)
            return pkg_registry[i].winget_id;
    }
    return name; /* pass through as-is */
}
#else
typedef enum { PM_APT, PM_BREW, PM_PACMAN, PM_NONE } pkg_manager_t;

static pkg_manager_t detect_unix_pm(void) {
    if (system("which apt-get >/dev/null 2>&1")  == 0) return PM_APT;
    if (system("which brew >/dev/null 2>&1")      == 0) return PM_BREW;
    if (system("which pacman >/dev/null 2>&1")    == 0) return PM_PACMAN;
    return PM_NONE;
}
#endif

/* ---- Sub-commands ---- */
static int pkg_install(const char *name) {
    char cmd[512];
    char info_msg[256];
    snprintf(info_msg, sizeof(info_msg), "Installing %s...", name);
    pkg_info(info_msg);

#ifdef _WIN32
    pkg_manager_t pm = detect_win_pm();
    const char *id = find_winget_id(name);
    switch (pm) {
        case PM_WINGET:
            snprintf(cmd, sizeof(cmd), "winget install --id \"%s\" -e", id);
            break;
        case PM_SCOOP:
            snprintf(cmd, sizeof(cmd), "scoop install %s", name);
            break;
        case PM_CHOCO:
            snprintf(cmd, sizeof(cmd), "choco install %s -y", name);
            break;
        default:
            pkg_err("No supported package manager found (winget, scoop, choco).");
            return 1;
    }
#else
    pkg_manager_t pm = detect_unix_pm();
    switch (pm) {
        case PM_APT:
            snprintf(cmd, sizeof(cmd), "sudo apt-get install -y %s", name);
            break;
        case PM_BREW:
            snprintf(cmd, sizeof(cmd), "brew install %s", name);
            break;
        case PM_PACMAN:
            snprintf(cmd, sizeof(cmd), "sudo pacman -S --noconfirm %s", name);
            break;
        default:
            pkg_err("No supported package manager found (apt, brew, pacman).");
            return 1;
    }
#endif

    int ret = system(cmd);
    if (ret == 0) {
        pkg_mark_installed(name);
        char ok_msg[256];
        snprintf(ok_msg, sizeof(ok_msg), "Successfully installed %s.", name);
        pkg_ok(ok_msg);
        return 0;
    } else {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Failed to install %s.", name);
        pkg_err(err_msg);
        return 1;
    }
}

static int pkg_remove(const char *name) {
    char cmd[512];
    char info_msg[256];
    snprintf(info_msg, sizeof(info_msg), "Removing %s...", name);
    pkg_info(info_msg);

#ifdef _WIN32
    pkg_manager_t pm = detect_win_pm();
    const char *id = find_winget_id(name);
    switch (pm) {
        case PM_WINGET:
            snprintf(cmd, sizeof(cmd), "winget uninstall --id \"%s\" -e", id);
            break;
        case PM_SCOOP:
            snprintf(cmd, sizeof(cmd), "scoop uninstall %s", name);
            break;
        case PM_CHOCO:
            snprintf(cmd, sizeof(cmd), "choco uninstall %s -y", name);
            break;
        default:
            pkg_err("No supported package manager found.");
            return 1;
    }
#else
    pkg_manager_t pm = detect_unix_pm();
    switch (pm) {
        case PM_APT:
            snprintf(cmd, sizeof(cmd), "sudo apt-get remove -y %s", name);
            break;
        case PM_BREW:
            snprintf(cmd, sizeof(cmd), "brew uninstall %s", name);
            break;
        case PM_PACMAN:
            snprintf(cmd, sizeof(cmd), "sudo pacman -R --noconfirm %s", name);
            break;
        default:
            pkg_err("No supported package manager found.");
            return 1;
    }
#endif

    int ret = system(cmd);
    if (ret == 0) {
        pkg_unmark_installed(name);
        char ok_msg[256];
        snprintf(ok_msg, sizeof(ok_msg), "Successfully removed %s.", name);
        pkg_ok(ok_msg);
        return 0;
    } else {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Failed to remove %s.", name);
        pkg_err(err_msg);
        return 1;
    }
}

static int pkg_search(const char *query) {
    printf("%sSearching for '%s'...%s\n", PKG_COLOR_CYAN, query, PKG_COLOR_RESET);
    printf("\n%s%-15s %-40s %s%s\n", PKG_COLOR_CYAN,
           "Name", "Description", "Installed", PKG_COLOR_RESET);
    printf("%-15s %-40s %s\n",
           "---------------", "----------------------------------------", "---------");

    bool found = false;
    for (int i = 0; pkg_registry[i].name; i++) {
        if (strstr(pkg_registry[i].name, query) ||
            strstr(pkg_registry[i].description, query)) {
            bool installed = pkg_is_installed(pkg_registry[i].name);
            printf("%-15s %-40s %s%s%s\n",
                   pkg_registry[i].name,
                   pkg_registry[i].description,
                   installed ? PKG_COLOR_GREEN : "",
                   installed ? "yes" : "no",
                   installed ? PKG_COLOR_RESET : "");
            found = true;
        }
    }
    if (!found)
        printf("No packages found matching '%s'.\n", query);
    return 0;
}

static int pkg_list(void) {
    char path[512];
    get_pkg_file(path, sizeof(path));
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("No packages installed via cFd pkg.\n");
        return 0;
    }
    printf("%sInstalled packages:%s\n", PKG_COLOR_CYAN, PKG_COLOR_RESET);
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (len > 0) {
            printf("  %s%s%s\n", PKG_COLOR_GREEN, line, PKG_COLOR_RESET);
            count++;
        }
    }
    fclose(fp);
    if (count == 0) printf("  (none)\n");
    return 0;
}

static int pkg_update(void) {
    pkg_info("Updating all packages...");
    char cmd[128];
#ifdef _WIN32
    pkg_manager_t pm = detect_win_pm();
    switch (pm) {
        case PM_WINGET: snprintf(cmd, sizeof(cmd), "winget upgrade --all"); break;
        case PM_SCOOP:  snprintf(cmd, sizeof(cmd), "scoop update *"); break;
        case PM_CHOCO:  snprintf(cmd, sizeof(cmd), "choco upgrade all -y"); break;
        default: pkg_err("No supported package manager found."); return 1;
    }
#else
    pkg_manager_t pm = detect_unix_pm();
    switch (pm) {
        case PM_APT:    snprintf(cmd, sizeof(cmd), "sudo apt-get upgrade -y"); break;
        case PM_BREW:   snprintf(cmd, sizeof(cmd), "brew upgrade"); break;
        case PM_PACMAN: snprintf(cmd, sizeof(cmd), "sudo pacman -Syu --noconfirm"); break;
        default: pkg_err("No supported package manager found."); return 1;
    }
#endif
    int ret = system(cmd);
    if (ret == 0) pkg_ok("Update complete.");
    else          pkg_err("Update failed.");
    return (ret == 0) ? 0 : 1;
}

static int pkg_info_cmd(const char *name) {
    for (int i = 0; pkg_registry[i].name; i++) {
        if (strcmp(pkg_registry[i].name, name) == 0) {
            printf("%sPackage: %s%s\n", PKG_COLOR_CYAN, name, PKG_COLOR_RESET);
            printf("  Description : %s\n", pkg_registry[i].description);
#ifdef _WIN32
            printf("  Winget ID   : %s\n", pkg_registry[i].winget_id);
#endif
            printf("  Installed   : %s\n", pkg_is_installed(name) ? "yes" : "no");
            return 0;
        }
    }
    fprintf(stderr, "pkg: package '%s' not found in registry.\n", name);
    return 1;
}

/* ---- Main entry point ---- */
int cmd_pkg(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    if (argc < 2) {
        printf("Usage: pkg <command> [arguments]\n\n");
        printf("Commands:\n");
        printf("  install <name>  Install a package\n");
        printf("  remove <name>   Remove a package\n");
        printf("  search <query>  Search packages\n");
        printf("  list            List installed packages\n");
        printf("  update          Update all packages\n");
        printf("  info <name>     Show package information\n");
        return 1;
    }

    const char *subcmd = argv[1];

    if (strcmp(subcmd, "install") == 0) {
        if (argc < 3) { fprintf(stderr, "pkg: install requires a package name\n"); return 1; }
        int ret = 0;
        for (int i = 2; i < argc; i++) ret |= pkg_install(argv[i]);
        return ret;
    }
    if (strcmp(subcmd, "remove") == 0 || strcmp(subcmd, "uninstall") == 0) {
        if (argc < 3) { fprintf(stderr, "pkg: remove requires a package name\n"); return 1; }
        int ret = 0;
        for (int i = 2; i < argc; i++) ret |= pkg_remove(argv[i]);
        return ret;
    }
    if (strcmp(subcmd, "search") == 0) {
        if (argc < 3) { fprintf(stderr, "pkg: search requires a query\n"); return 1; }
        return pkg_search(argv[2]);
    }
    if (strcmp(subcmd, "list") == 0) {
        return pkg_list();
    }
    if (strcmp(subcmd, "update") == 0 || strcmp(subcmd, "upgrade") == 0) {
        return pkg_update();
    }
    if (strcmp(subcmd, "info") == 0) {
        if (argc < 3) { fprintf(stderr, "pkg: info requires a package name\n"); return 1; }
        return pkg_info_cmd(argv[2]);
    }

    fprintf(stderr, "pkg: unknown command '%s'\n", subcmd);
    return 1;
}

const cfd_command_t builtin_pkg = {
    "pkg",
    "pkg <install|remove|search|list|update|info> [args]",
    "Package manager - install and manage software",
    "pkg",
    cmd_pkg,
    1, -1
};
