#ifndef CFD_PLATFORM_COMMON_H
#define CFD_PLATFORM_COMMON_H

/* Shared utilities across platforms */
void cfd_platform_init(void);
void cfd_platform_shutdown(void);

const char *cfd_platform_name(void);
const char *cfd_platform_newline(void);

#endif /* CFD_PLATFORM_COMMON_H */
