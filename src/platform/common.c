#include "common.h"
#include "platform.h"
#include <stdio.h>

void cfd_platform_init(void) {
#ifdef CFD_PLATFORM_WINDOWS
    cfd_platform_enable_ansi();
#endif
}

void cfd_platform_shutdown(void) {
    cfd_platform_raw_mode_exit();
}

const char *cfd_platform_name(void) {
    return CFD_PLATFORM_NAME;
}

const char *cfd_platform_newline(void) {
#ifdef CFD_PLATFORM_WINDOWS
    return "\r\n";
#else
    return "\n";
#endif
}
