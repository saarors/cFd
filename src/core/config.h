#ifndef CFD_CORE_CONFIG_H
#define CFD_CORE_CONFIG_H

#include "../../include/types.h"
#include <stdbool.h>

typedef struct cfd_config {
    char  prompt_format[256];
    char  theme[64];
    bool  color_enabled;
    bool  history_enabled;
    bool  completion_enabled;
    int   history_size;
    char  histfile[512];
    char  rcfile[512];
    bool  show_banner;
    bool  vi_mode;
} cfd_config_t;

cfd_config_t *cfd_config_new(void);
void          cfd_config_free(cfd_config_t *cfg);
int           cfd_config_load(cfd_config_t *cfg, const char *path);
int           cfd_config_save(cfd_config_t *cfg, const char *path);
void          cfd_config_apply(cfd_config_t *cfg, cfd_session_t *sess);

extern cfd_config_t *g_config;

#endif /* CFD_CORE_CONFIG_H */
