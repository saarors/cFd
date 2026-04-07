#include "cmd_version.h"
#include "../../../../include/version.h"
#include "../../../platform/platform.h"
#include <stdio.h>
int cmd_version(cfd_session_t *sess, int argc, char **argv) {
    (void)sess;(void)argc;(void)argv;
    printf("%s %s (%s %s)\n", CFD_NAME, CFD_VERSION, CFD_BUILD_DATE, CFD_BUILD_TIME);
    printf("Platform: %s\n", cfd_platform_name());
    return 0;
}
const cfd_command_t builtin_version = {
    "version","version","Show cFd version","system",cmd_version,0,0
};
