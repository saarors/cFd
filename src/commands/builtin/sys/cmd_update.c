#include "cmd_update.h"
#include "../../../../include/version.h"
#include "../../../utils/mem.h"
#include "../../../utils/str_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
#  include <windows.h>
#  include <winhttp.h>
#  include <shlobj.h>
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <errno.h>
#  if defined(__APPLE__)
#    include <mach-o/dyld.h>
#  endif
#endif

#define UPDATE_API_HOST  "api.github.com"
#define UPDATE_API_PATH  "/repos/saarors/cFd/releases/latest"
#define UPDATE_API_URL   "https://api.github.com/repos/saarors/cFd/releases/latest"
#define REPO_OWNER       "saarors"
#define REPO_NAME        "cFd"

/* How long to use a cached check result before re-fetching (seconds) */
#define UPDATE_CACHE_TTL  (60 * 60 * 24)   /* 24 hours */

/* ── Utilities ───────────────────────────────────────────────────────────── */

static const char *get_home(void) {
#ifdef _WIN32
    static char home[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home)))
        return home;
    const char *up = getenv("USERPROFILE");
    return up ? up : "C:\\";
#else
    const char *h = getenv("HOME");
    return h ? h : "/tmp";
#endif
}

static void ensure_cfd_dir(void) {
    char dir[1024];
    snprintf(dir, sizeof(dir), "%s/.cfd", get_home());
#ifdef _WIN32
    CreateDirectoryA(dir, NULL);
#else
    mkdir(dir, 0700);
#endif
}

static void cache_path(char *out, size_t size) {
    snprintf(out, size, "%s/.cfd/update_cache", get_home());
}

/* ── Version comparison ──────────────────────────────────────────────────── */

static void parse_ver(const char *s, int *maj, int *min, int *pat) {
    if (s && *s == 'v') s++;
    *maj = *min = *pat = 0;
    if (s) sscanf(s, "%d.%d.%d", maj, min, pat);
}

static bool version_newer_than_current(const char *remote_tag) {
    int rmaj, rmin, rpat;
    parse_ver(remote_tag, &rmaj, &rmin, &rpat);
    if (rmaj != CFD_MAJOR) return rmaj > CFD_MAJOR;
    if (rmin != CFD_MINOR) return rmin > CFD_MINOR;
    return rpat > CFD_PATCH;
}

/* ── Minimal JSON string extractor ──────────────────────────────────────── */

/* Returns malloc'd value for the first occurrence of "key": "value" */
static char *json_str(const char *json, const char *key) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = json;
    while ((p = strstr(p, needle)) != NULL) {
        p += strlen(needle);
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (*p != ':') continue;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (*p != '"') continue;
        p++;
        char buf[8192];
        size_t bi = 0;
        while (*p && *p != '"' && bi < sizeof(buf) - 1) {
            if (*p == '\\' && p[1]) {
                p++;
                switch (*p) {
                    case 'n':  buf[bi++] = '\n'; break;
                    case 't':  buf[bi++] = '\t'; break;
                    case 'r':  buf[bi++] = '\r'; break;
                    case '"':  buf[bi++] = '"';  break;
                    case '\\': buf[bi++] = '\\'; break;
                    default:   buf[bi++] = *p;   break;
                }
            } else {
                buf[bi++] = *p;
            }
            p++;
        }
        buf[bi] = '\0';
        return strdup(buf);
    }
    return NULL;
}

/*
 * Find the browser_download_url for an asset whose name contains `suffix`.
 * Scans the "assets" array in the JSON.
 */
static char *json_asset_url(const char *json, const char *suffix) {
    const char *assets = strstr(json, "\"assets\"");
    if (!assets) return NULL;
    assets = strchr(assets, '[');
    if (!assets) return NULL;

    const char *p = assets;
    while ((p = strstr(p, "\"name\"")) != NULL) {
        char *name = json_str(p, "name");
        if (!name) { p++; continue; }
        bool match = (strstr(name, suffix) != NULL);
        free(name);
        if (match) {
            /* get browser_download_url in this same object */
            /* find the next '}' to bound our search */
            const char *obj_end = strchr(p, '}');
            if (!obj_end) break;
            /* search between p and obj_end */
            char slice[4096];
            size_t slen = (size_t)(obj_end - p);
            if (slen >= sizeof(slice)) slen = sizeof(slice) - 1;
            memcpy(slice, p, slen);
            slice[slen] = '\0';
            char *url = json_str(slice, "browser_download_url");
            if (url) return url;
        }
        p++;
    }
    return NULL;
}

/* ── HTTP GET → malloc'd body (NULL on failure) ──────────────────────────── */

#ifdef _WIN32
static char *https_get(const char *host, const char *path) {
    HINTERNET hSession = WinHttpOpen(
        L"cFd-updater/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return NULL;

    wchar_t whost[256];
    MultiByteToWideChar(CP_UTF8, 0, host, -1, whost, 256);
    HINTERNET hConn = WinHttpConnect(hSession, whost, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConn) { WinHttpCloseHandle(hSession); return NULL; }

    wchar_t wpath[1024];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 1024);
    HINTERNET hReq = WinHttpOpenRequest(
        hConn, L"GET", wpath, NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hReq) { WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession); return NULL; }

    /* Accept redirects; relax SSL (GitHub is fine, but avoids cert issues) */
    DWORD redirect = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hReq, WINHTTP_OPTION_REDIRECT_POLICY, &redirect, sizeof(redirect));

    WinHttpAddRequestHeaders(hReq,
        L"Accept: application/vnd.github+json\r\n"
        L"X-GitHub-Api-Version: 2022-11-28\r\n",
        (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

    if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0) ||
        !WinHttpReceiveResponse(hReq, NULL)) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession);
        return NULL;
    }

    size_t cap = 65536, len = 0;
    char *buf = malloc(cap);
    if (!buf) { WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession); return NULL; }

    DWORD avail, nread;
    while (WinHttpQueryDataAvailable(hReq, &avail) && avail > 0) {
        if (len + avail + 1 > cap) {
            cap = (len + avail + 1) * 2;
            char *nb = realloc(buf, cap);
            if (!nb) { free(buf); buf = NULL; break; }
            buf = nb;
        }
        if (!WinHttpReadData(hReq, buf + len, avail, &nread)) break;
        len += nread;
    }
    if (buf) buf[len] = '\0';

    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConn); WinHttpCloseHandle(hSession);
    return buf;
}

/* Download URL to a local file path using WinHTTP (follows redirects) */
static int download_to_file(const char *url, const char *dest) {
    /* Parse URL: https://host/path */
    const char *after_scheme = strstr(url, "://");
    if (!after_scheme) return 1;
    after_scheme += 3;
    const char *path_start = strchr(after_scheme, '/');
    if (!path_start) return 1;

    char host[256];
    size_t hlen = (size_t)(path_start - after_scheme);
    if (hlen >= sizeof(host)) return 1;
    strncpy(host, after_scheme, hlen);
    host[hlen] = '\0';

    /* Use PowerShell for reliable HTTPS download with redirect following */
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "powershell -NoProfile -NonInteractive -Command "
        "\"Invoke-WebRequest -Uri '%s' -OutFile '%s' -UseBasicParsing\"",
        url, dest);
    return system(cmd);
}

#else /* Unix */

static char *https_get(const char *host, const char *path) {
    (void)host;
    char url[512];
    snprintf(url, sizeof(url), "https://%s%s", host, path);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "curl -s -L "
        "-H 'Accept: application/vnd.github+json' "
        "-H 'X-GitHub-Api-Version: 2022-11-28' "
        "-A 'cFd/%s' "
        "'%s' 2>/dev/null",
        CFD_VERSION, url);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        /* fallback: wget */
        snprintf(cmd, sizeof(cmd),
            "wget -q -O - "
            "--header='Accept: application/vnd.github+json' "
            "--user-agent='cFd/%s' "
            "'%s' 2>/dev/null",
            CFD_VERSION, url);
        fp = popen(cmd, "r");
    }
    if (!fp) return NULL;

    size_t cap = 65536, len = 0;
    char *buf = malloc(cap);
    if (!buf) { pclose(fp); return NULL; }
    char tmp[4096];
    size_t n;
    while ((n = fread(tmp, 1, sizeof(tmp), fp)) > 0) {
        if (len + n + 1 > cap) {
            cap = (len + n + 1) * 2;
            char *nb = realloc(buf, cap);
            if (!nb) { free(buf); pclose(fp); return NULL; }
            buf = nb;
        }
        memcpy(buf + len, tmp, n);
        len += n;
    }
    buf[len] = '\0';
    pclose(fp);
    return buf;
}

static int download_to_file(const char *url, const char *dest) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "curl -s -L -A 'cFd/%s' '%s' -o '%s' 2>/dev/null",
        CFD_VERSION, url, dest);
    if (system(cmd) == 0) return 0;
    snprintf(cmd, sizeof(cmd),
        "wget -q --user-agent='cFd/%s' '%s' -O '%s' 2>/dev/null",
        CFD_VERSION, url, dest);
    return system(cmd);
}

#endif /* _WIN32 */

/* ── Current executable path ─────────────────────────────────────────────── */

#ifndef _WIN32
static int get_exe_path(char *out, size_t size) {
#  if defined(__APPLE__)
    uint32_t sz = (uint32_t)size;
    if (_NSGetExecutablePath(out, &sz) == 0) return 0;
    return 1;
#  else
    ssize_t n = readlink("/proc/self/exe", out, size - 1);
    if (n > 0) { out[n] = '\0'; return 0; }
    return 1;
#  endif
}
#endif /* !_WIN32 */

/* ── Platform asset suffix ───────────────────────────────────────────────── */

static const char *asset_suffix(void) {
#ifdef _WIN32
    return "-windows.msi";
#elif defined(__APPLE__)
    return "-macos.tar.gz";
#else
    return "-linux-x86_64.tar.gz";
#endif
}

/* ── Cache file: stores "UNIX_TIMESTAMP\nLATEST_TAG\n" ──────────────────── */

static void cache_write(const char *tag) {
    ensure_cfd_dir();
    char path[1024];
    cache_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%ld\n%s\n", (long)time(NULL), tag);
    fclose(f);
}

/* Returns 1 if cached and fresh, writes tag into out_tag */
static int cache_read(char *out_tag, size_t tag_size) {
    char path[1024];
    cache_path(path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    long ts = 0;
    char tag[64] = "";
    int ok = (fscanf(f, "%ld\n%63s\n", &ts, tag) == 2);
    fclose(f);
    if (!ok) return 0;
    if ((long)time(NULL) - ts > UPDATE_CACHE_TTL) return 0; /* stale */
    strncpy(out_tag, tag, tag_size - 1);
    out_tag[tag_size - 1] = '\0';
    return 1;
}

/* ── Fetch latest release tag from GitHub API ────────────────────────────── */

static char *fetch_latest_tag(void) {
    char *body = https_get(UPDATE_API_HOST, UPDATE_API_PATH);
    if (!body) return NULL;
    char *tag = json_str(body, "tag_name");
    free(body);
    return tag;
}

/* ── Self-update: download + replace binary ──────────────────────────────── */

static int do_self_update(const char *tag, bool silent) {
    /* First fetch the full release JSON to get asset URLs */
    char api_path[256];
    snprintf(api_path, sizeof(api_path),
             "/repos/%s/%s/releases/tags/%s", REPO_OWNER, REPO_NAME, tag);
    char *body = https_get(UPDATE_API_HOST, api_path);
    if (!body) {
        fprintf(stderr, "update: failed to fetch release info\n");
        return 1;
    }

    const char *suf = asset_suffix();
    char *dl_url = json_asset_url(body, suf);
    free(body);

    if (!dl_url) {
        fprintf(stderr,
            "update: no asset found for this platform (%s).\n"
            "  Download manually: https://github.com/%s/%s/releases/tag/%s\n",
            suf, REPO_OWNER, REPO_NAME, tag);
        return 1;
    }

    if (!silent)
        printf("  Downloading %s...\n", dl_url);

#ifdef _WIN32
    /* Download MSI to temp, run installer, exit */
    char tmp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, tmp_path);
    char msi_path[MAX_PATH];
    snprintf(msi_path, sizeof(msi_path), "%scFd-%s.msi", tmp_path, tag);

    if (download_to_file(dl_url, msi_path) != 0) {
        fprintf(stderr, "update: download failed\n");
        free(dl_url);
        return 1;
    }
    free(dl_url);

    printf("  Running installer: %s\n", msi_path);
    printf("  cFd will close now. Restart after installation completes.\n");
    fflush(stdout);

    char install_cmd[MAX_PATH + 64];
    snprintf(install_cmd, sizeof(install_cmd), "msiexec /i \"%s\"", msi_path);
    ShellExecuteA(NULL, "open", "msiexec", install_cmd + 9, NULL, SW_SHOW);
    exit(0);

#elif defined(__APPLE__)
    /* Download tar.gz, extract cFd binary, replace current exe */
    char tmp_tgz[]  = "/tmp/cfd_update.tar.gz";
    if (download_to_file(dl_url, tmp_tgz) != 0) {
        fprintf(stderr, "update: download failed\n");
        free(dl_url);
        return 1;
    }
    free(dl_url);

    char exe_path[1024] = "";
    if (get_exe_path(exe_path, sizeof(exe_path)) != 0) {
        fprintf(stderr, "update: could not determine current executable path\n");
        return 1;
    }

    /* Extract: the tarball might contain cFd.app or cfd binary */
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "cd /tmp && tar xzf cfd_update.tar.gz 2>/dev/null; "
        "if [ -f /tmp/cfd ]; then "
        "  chmod +x /tmp/cfd && cp /tmp/cfd '%s' && rm -f /tmp/cfd; "
        "fi",
        exe_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "update: extraction/replace failed. Try: sudo cp /tmp/cfd %s\n", exe_path);
        return 1;
    }
    printf("  Updated to %s. Please restart cFd.\n", tag);

#else /* Linux */
    char tmp_tgz[] = "/tmp/cfd_update.tar.gz";
    if (download_to_file(dl_url, tmp_tgz) != 0) {
        fprintf(stderr, "update: download failed\n");
        free(dl_url);
        return 1;
    }
    free(dl_url);

    char exe_path[1024] = "";
    if (get_exe_path(exe_path, sizeof(exe_path)) != 0) {
        fprintf(stderr, "update: could not determine current executable path\n");
        return 1;
    }

    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "tar xzf /tmp/cfd_update.tar.gz -C /tmp/ cfd 2>/dev/null && "
        "chmod +x /tmp/cfd && "
        "cp /tmp/cfd '%s' && "
        "rm -f /tmp/cfd /tmp/cfd_update.tar.gz",
        exe_path);
    if (system(cmd) != 0) {
        fprintf(stderr,
            "update: replacement failed (permissions?). "
            "Try: sudo cp /tmp/cfd %s\n", exe_path);
        return 1;
    }
    printf("\033[32m  Updated to %s. Please restart cFd.\033[0m\n", tag);
#endif

    return 0;
}

/* ── Public startup notification ─────────────────────────────────────────── */

int cfd_update_notify_startup(void) {
    char cached_tag[64] = "";

    if (cache_read(cached_tag, sizeof(cached_tag))) {
        /* Use cached result */
        if (version_newer_than_current(cached_tag)) {
            printf("\033[33m"
                   "  A new version of cFd is available: %s  "
                   "(current: v" CFD_VERSION ")\n"
                   "  Run \033[1mupdate\033[0;33m to install it.\033[0m\n",
                   cached_tag);
            return 1;
        }
        return 0;
    }

    /* Cache is stale — do a background fetch so startup isn't blocked */
#ifndef _WIN32
    pid_t pid = fork();
    if (pid == 0) {
        /* Child: fetch and write cache, then exit silently */
        char *tag = fetch_latest_tag();
        if (tag) {
            cache_write(tag);
            free(tag);
        }
        _exit(0);
    }
    /* Parent continues immediately */
    (void)pid;
#else
    /* On Windows, do a quick synchronous check (WinHTTP is fast) */
    char *tag = fetch_latest_tag();
    if (tag) {
        cache_write(tag);
        if (version_newer_than_current(tag)) {
            printf("\033[33m"
                   "  A new version of cFd is available: %s  "
                   "(current: v" CFD_VERSION ")\n"
                   "  Run \033[1mupdate\033[0;33m to install it.\033[0m\n",
                   tag);
            free(tag);
            return 1;
        }
        free(tag);
    }
#endif

    return 0;
}

/* ── update command ──────────────────────────────────────────────────────── */

int cmd_update(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool check_only = false;
    bool auto_yes   = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--check") == 0 || strcmp(argv[i], "-c") == 0)
            check_only = true;
        else if (strcmp(argv[i], "--yes") == 0 || strcmp(argv[i], "-y") == 0)
            auto_yes = true;
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: update [--check] [--yes]\n"
                   "  --check   Only check for updates, do not install\n"
                   "  --yes     Install without prompting\n");
            return 0;
        }
    }

    printf("Checking for updates...\n");
    fflush(stdout);

    /* Fetch latest release info */
    char *json = https_get(UPDATE_API_HOST, UPDATE_API_PATH);
    if (!json) {
        fprintf(stderr, "update: could not reach GitHub API. Check your connection.\n");
        return 1;
    }

    char *tag  = json_str(json, "tag_name");
    char *body = json_str(json, "body");
    free(json);

    if (!tag) {
        fprintf(stderr, "update: failed to parse release info\n");
        free(body);
        return 1;
    }

    /* Update cache */
    cache_write(tag);

    if (!version_newer_than_current(tag)) {
        printf("\033[32mcFd is up to date (v" CFD_VERSION ").\033[0m\n");
        free(tag);
        free(body);
        return 0;
    }

    /* Show what's new */
    printf("\033[1;36mNew version available: %s\033[0m  (current: v" CFD_VERSION ")\n\n", tag);
    if (body && *body) {
        printf("\033[1mWhat's new:\033[0m\n");
        /* Print each line of the release body */
        const char *p = body;
        while (*p) {
            const char *nl = strchr(p, '\n');
            size_t len = nl ? (size_t)(nl - p) : strlen(p);
            printf("  %.*s\n", (int)len, p);
            if (!nl) break;
            p = nl + 1;
        }
        printf("\n");
    }
    free(body);

    if (check_only) {
        printf("Run \033[1mupdate\033[0m to install.\n");
        free(tag);
        return 0;
    }

    if (!auto_yes) {
        printf("Install %s? [y/N] ", tag);
        fflush(stdout);
        char ans[8] = "";
        if (fgets(ans, sizeof(ans), stdin) == NULL ||
            (ans[0] != 'y' && ans[0] != 'Y')) {
            printf("Cancelled.\n");
            free(tag);
            return 0;
        }
    }

    int ret = do_self_update(tag, false);
    free(tag);
    return ret;
}

const cfd_command_t builtin_update = {
    "update",
    "update [--check] [--yes]",
    "Check for and install cFd updates",
    "system",
    cmd_update,
    0, 2
};
