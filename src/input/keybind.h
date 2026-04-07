#ifndef CFD_KEYBIND_H
#define CFD_KEYBIND_H

/* Key codes */
#define KEY_CTRL(c)  ((c) - '@')
#define KEY_UP       0x100
#define KEY_DOWN     0x101
#define KEY_LEFT     0x102
#define KEY_RIGHT    0x103
#define KEY_HOME     0x104
#define KEY_END      0x105
#define KEY_DELETE   0x106
#define KEY_BACKSPACE 0x7F
#define KEY_ENTER    0x0D
#define KEY_TAB      0x09
#define KEY_ESC      0x1B
#define KEY_PGUP     0x107
#define KEY_PGDN     0x108
#define KEY_F1       0x110
#define KEY_F2       0x111
#define KEY_F3       0x112
#define KEY_F4       0x113
#define KEY_F5       0x114
#define KEY_F6       0x115
#define KEY_F7       0x116
#define KEY_F8       0x117
#define KEY_F9       0x118
#define KEY_F10      0x119
#define KEY_F11      0x11A
#define KEY_F12      0x11B

typedef void (*keybind_action_t)(void *ctx);

typedef struct cfd_keybind {
    int              key;
    keybind_action_t action;
    void            *ctx;
    const char      *description;
} cfd_keybind_t;

void cfd_keybind_init(void);
void cfd_keybind_register(int key, keybind_action_t action, void *ctx, const char *desc);
keybind_action_t cfd_keybind_lookup(int key, void **ctx_out);

/* Read a key (handles escape sequences) */
int cfd_read_key(void);

#endif /* CFD_KEYBIND_H */
