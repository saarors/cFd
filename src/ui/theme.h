#ifndef CFD_THEME_H
#define CFD_THEME_H

#include "color.h"

typedef struct cfd_theme {
    const char    *name;
    cfd_color_id_t prompt_user;
    cfd_color_id_t prompt_host;
    cfd_color_id_t prompt_path;
    cfd_color_id_t prompt_symbol;
    cfd_color_id_t error_color;
    cfd_color_id_t success_color;
    cfd_color_id_t info_color;
    cfd_color_id_t warn_color;
    cfd_color_id_t dir_color;
    cfd_color_id_t file_color;
    cfd_color_id_t exec_color;
    cfd_color_id_t link_color;
} cfd_theme_t;

void              cfd_theme_init(void);
const cfd_theme_t *cfd_theme_get(void);
void              cfd_theme_set(const char *name);

/* Built-in themes */
extern const cfd_theme_t cfd_theme_default;
extern const cfd_theme_t cfd_theme_dark;
extern const cfd_theme_t cfd_theme_minimal;

#endif /* CFD_THEME_H */
