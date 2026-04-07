#include "config.h"
#include "session.h"
#include "../utils/mem.h"
#include "../utils/str_utils.h"
#include "../platform/platform.h"
#include "../ui/theme.h"
#include "../ui/color.h"
#include "../ui/prompt.h"
#include "../../include/config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

cfd_config_t *g_config = NULL;

cfd_config_t *cfd_config_new(void) {
    cfd_config_t *cfg = CFD_NEW(cfd_config_t);
    strncpy(cfg->prompt_format, "\\u@\\h \\w> ", sizeof(cfg->prompt_format)-1);
    strncpy(cfg->theme, CFD_THEME_DEFAULT, sizeof(cfg->theme)-1);
    cfg->color_enabled      = true;
    cfg->history_enabled    = true;
    cfg->completion_enabled = true;
    cfg->history_size       = CFD_MAX_HISTORY;
    cfg->show_banner        = true;
    cfg->vi_mode            = false;

    char *home = cfd_platform_get_home();
    snprintf(cfg->histfile, sizeof(cfg->histfile), "%s/%s", home, CFD_HISTFILE);
    snprintf(cfg->rcfile,   sizeof(cfg->rcfile),   "%s/%s", home, CFD_RCFILE);
    cfd_free(home);
    return cfg;
}

void cfd_config_free(cfd_config_t *cfg) { cfd_free(cfg); }

int cfd_config_load(cfd_config_t *cfg, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *s = cfd_strtrim(line);
        if (!*s || *s == '#') continue;
        char *eq = strchr(s, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = cfd_strtrim(s);
        char *val = cfd_strtrim(eq + 1);
        if (strcmp(key,"prompt")==0)    strncpy(cfg->prompt_format,val,sizeof(cfg->prompt_format)-1);
        else if (strcmp(key,"theme")==0) strncpy(cfg->theme,val,sizeof(cfg->theme)-1);
        else if (strcmp(key,"color")==0) cfg->color_enabled=(strcmp(val,"true")==0||strcmp(val,"1")==0);
        else if (strcmp(key,"banner")==0) cfg->show_banner=(strcmp(val,"true")==0||strcmp(val,"1")==0);
        else if (strcmp(key,"history_size")==0) cfg->history_size=atoi(val);
    }
    fclose(f);
    return 0;
}

int cfd_config_save(cfd_config_t *cfg, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f,"prompt=%s\n",cfg->prompt_format);
    fprintf(f,"theme=%s\n",cfg->theme);
    fprintf(f,"color=%s\n",cfg->color_enabled?"true":"false");
    fprintf(f,"banner=%s\n",cfg->show_banner?"true":"false");
    fprintf(f,"history_size=%d\n",cfg->history_size);
    fclose(f);
    return 0;
}

void cfd_config_apply(cfd_config_t *cfg, cfd_session_t *sess) {
    (void)sess;
    cfd_prompt_set_format(cfg->prompt_format);
    cfd_theme_set(cfg->theme);
    cfd_color_set_enabled(cfg->color_enabled);
}
