#ifndef CFD_LOG_H
#define CFD_LOG_H

#include "../../include/types.h"

void cfd_log_init(const char *logfile, cfd_log_level_t min_level);
void cfd_log_close(void);
void cfd_log(cfd_log_level_t level, const char *fmt, ...);
void cfd_log_set_level(cfd_log_level_t level);

#define CFD_LOG_DEBUG(...)  cfd_log(CFD_LOG_DEBUG, __VA_ARGS__)
#define CFD_LOG_INFO(...)   cfd_log(CFD_LOG_INFO,  __VA_ARGS__)
#define CFD_LOG_WARN(...)   cfd_log(CFD_LOG_WARN,  __VA_ARGS__)
#define CFD_LOG_ERROR(...)  cfd_log(CFD_LOG_ERROR, __VA_ARGS__)
#define CFD_LOG_FATAL(...)  cfd_log(CFD_LOG_FATAL, __VA_ARGS__)

#endif /* CFD_LOG_H */
