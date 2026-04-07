#ifndef CFD_ERROR_H
#define CFD_ERROR_H

#include "../../include/types.h"

/* Set/get last error */
void        cfd_set_error(cfd_err_t code, const char *fmt, ...);
cfd_err_t   cfd_get_error_code(void);
const char *cfd_get_error_msg(void);
void        cfd_clear_error(void);

/* Print error to stderr */
void cfd_perror(const char *prefix);

/* Panic (non-recoverable) */
void cfd_panic(const char *fmt, ...) __attribute__((noreturn));

#endif /* CFD_ERROR_H */
