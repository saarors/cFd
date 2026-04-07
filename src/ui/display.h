#ifndef CFD_DISPLAY_H
#define CFD_DISPLAY_H

#include "color.h"
#include <stdbool.h>

/* Cursor movement */
void display_cursor_up(int n);
void display_cursor_down(int n);
void display_cursor_right(int n);
void display_cursor_left(int n);
void display_cursor_move(int row, int col);
void display_cursor_save(void);
void display_cursor_restore(void);
void display_cursor_show(bool show);

/* Erase */
void display_erase_line(void);
void display_erase_to_end(void);
void display_erase_to_start(void);
void display_erase_screen(void);

/* Output helpers */
void display_print(const char *s);
void display_println(const char *s);
void display_printf(const char *fmt, ...);
void display_error(const char *fmt, ...);
void display_warn(const char *fmt, ...);
void display_info(const char *fmt, ...);
void display_success(const char *fmt, ...);

/* Separator/divider */
void display_divider(char ch, int width);
void display_banner(const char *text);

/* Get terminal dimensions */
void display_get_size(int *cols, int *rows);

#endif /* CFD_DISPLAY_H */
