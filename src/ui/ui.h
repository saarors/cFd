#ifndef CFD_UI_H
#define CFD_UI_H

#include "color.h"
#include "theme.h"
#include "display.h"
#include "prompt.h"
#include "../../include/types.h"

void cfd_ui_init(void);
void cfd_ui_shutdown(void);

/* Startup banner */
void cfd_ui_print_banner(void);

/* Status line */
void cfd_ui_set_title(const char *title);

/* Pager */
int  cfd_ui_pager(const char *text);

#endif /* CFD_UI_H */
