#ifndef CFD_PROMPT_H
#define CFD_PROMPT_H

#include "../../include/types.h"
#include <stddef.h>

/* Build and render the prompt string */
char *cfd_prompt_build(cfd_session_t *sess);
void  cfd_prompt_print(cfd_session_t *sess);

/* Set custom prompt format */
void  cfd_prompt_set_format(const char *fmt);
const char *cfd_prompt_get_format(void);

/* Prompt segments */
char *cfd_prompt_segment_user(void);
char *cfd_prompt_segment_host(void);
char *cfd_prompt_segment_cwd(cfd_session_t *sess);
char *cfd_prompt_segment_git(void);
char *cfd_prompt_segment_status(int last_exit);

#endif /* CFD_PROMPT_H */
