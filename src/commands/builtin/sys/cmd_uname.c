#include "cmd_uname.h"
#include "../../../platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/utsname.h>
#endif

int cmd_uname(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;

    bool flag_a = false, flag_s = false, flag_r = false;
    bool flag_m = false, flag_n = false, flag_v = false;
    bool any    = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                switch (argv[i][j]) {
                    case 'a': flag_a = true; any = true; break;
                    case 's': flag_s = true; any = true; break;
                    case 'r': flag_r = true; any = true; break;
                    case 'm': flag_m = true; any = true; break;
                    case 'n': flag_n = true; any = true; break;
                    case 'v': flag_v = true; any = true; break;
                    default: break;
                }
            }
        }
    }

    /* Default: -s */
    if (!any) flag_s = true;

#ifdef _WIN32
    char sysname[64]  = "Windows";
    char nodename[256] = "unknown";
    char release[64]  = "unknown";
    char version[128] = "unknown";
    char machine[64]  = "unknown";

    /* Get hostname */
    DWORD sz = sizeof(nodename);
    GetComputerNameA(nodename, &sz);

    /* Get OS version */
    OSVERSIONINFOEXA osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
#pragma warning(push)
#pragma warning(disable:4996)
    if (GetVersionExA((LPOSVERSIONINFOA)&osvi)) {
        snprintf(release, sizeof(release), "%lu.%lu.%lu",
                 osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
        snprintf(version, sizeof(version), "Build %lu SP%d.%d",
                 osvi.dwBuildNumber, osvi.wServicePackMajor, osvi.wServicePackMinor);
    }
#pragma warning(pop)

    /* Machine arch */
    const char *proc_arch = getenv("PROCESSOR_ARCHITECTURE");
    if (proc_arch) {
        if (strcmp(proc_arch, "AMD64") == 0) strncpy(machine, "x86_64", sizeof(machine) - 1);
        else if (strcmp(proc_arch, "x86") == 0) strncpy(machine, "i686", sizeof(machine) - 1);
        else if (strcmp(proc_arch, "ARM64") == 0) strncpy(machine, "aarch64", sizeof(machine) - 1);
        else strncpy(machine, proc_arch, sizeof(machine) - 1);
    }
#else
    struct utsname u;
    if (uname(&u) != 0) {
        perror("uname");
        return 1;
    }
    char *sysname  = u.sysname;
    char *nodename = u.nodename;
    char *release  = u.release;
    char *version  = u.version;
    char *machine  = u.machine;
#endif

    bool printed = false;
    if (flag_a || flag_s) { if (printed) putchar(' '); fputs(sysname,  stdout); printed = true; }
    if (flag_a || flag_n) { if (printed) putchar(' '); fputs(nodename, stdout); printed = true; }
    if (flag_a || flag_r) { if (printed) putchar(' '); fputs(release,  stdout); printed = true; }
    if (flag_a || flag_v) { if (printed) putchar(' '); fputs(version,  stdout); printed = true; }
    if (flag_a || flag_m) { if (printed) putchar(' '); fputs(machine,  stdout); printed = true; }

    if (printed) putchar('\n');
    return 0;
}

const cfd_command_t builtin_uname = {
    "uname",
    "uname [-a|-s|-r|-m|-n|-v]",
    "Print system information",
    "system",
    cmd_uname,
    0, -1
};
