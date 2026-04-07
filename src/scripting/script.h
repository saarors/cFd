#ifndef CFD_SCRIPT_H
#define CFD_SCRIPT_H

#include "../../include/types.h"

/* Run a script file */
int cfd_script_run_file(cfd_session_t *sess, const char *path);

/* Run a string as a script */
int cfd_script_run_string(cfd_session_t *sess, const char *code);

/* Run the rc/init file */
int cfd_script_run_rc(cfd_session_t *sess);

#endif /* CFD_SCRIPT_H */
