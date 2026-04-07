#include "keybind.h"
#include "../platform/platform.h"
#include "../utils/mem.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>

#define MAX_KEYBINDS 128

static cfd_keybind_t g_binds[MAX_KEYBINDS];
static int           g_bind_count = 0;

void cfd_keybind_init(void) {
    g_bind_count = 0;
    memset(g_binds, 0, sizeof(g_binds));
}

void cfd_keybind_register(int key, keybind_action_t action, void *ctx, const char *desc) {
    if (g_bind_count >= MAX_KEYBINDS) return;
    for (int i = 0; i < g_bind_count; i++) {
        if (g_binds[i].key == key) {
            g_binds[i].action      = action;
            g_binds[i].ctx         = ctx;
            g_binds[i].description = desc;
            return;
        }
    }
    g_binds[g_bind_count].key         = key;
    g_binds[g_bind_count].action      = action;
    g_binds[g_bind_count].ctx         = ctx;
    g_binds[g_bind_count].description = desc;
    g_bind_count++;
}

keybind_action_t cfd_keybind_lookup(int key, void **ctx_out) {
    for (int i = 0; i < g_bind_count; i++) {
        if (g_binds[i].key == key) {
            if (ctx_out) *ctx_out = g_binds[i].ctx;
            return g_binds[i].action;
        }
    }
    return NULL;
}

int cfd_read_key(void) {
    int c = cfd_platform_read_key();
    if (c < 0) return -1;

#ifdef _WIN32
    /* Windows _getch returns 0 or 0xE0 for extended keys */
    if (c == 0 || c == 0xE0) {
        int c2 = cfd_platform_read_key();
        switch (c2) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 71: return KEY_HOME;
            case 79: return KEY_END;
            case 83: return KEY_DELETE;
            case 73: return KEY_PGUP;
            case 81: return KEY_PGDN;
            default: return c2 + 0x200;
        }
    }
    if (c == '\r') return KEY_ENTER;
    if (c == '\b') return KEY_BACKSPACE;
    return c;
#else
    if (c != KEY_ESC) return c;
    /* escape sequence */
    int c2 = cfd_platform_read_key();
    if (c2 != '[' && c2 != 'O') return KEY_ESC;
    int c3 = cfd_platform_read_key();
    switch (c3) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        case 'H': return KEY_HOME;
        case 'F': return KEY_END;
        case '3': { cfd_platform_read_key(); return KEY_DELETE; }
        case '5': { cfd_platform_read_key(); return KEY_PGUP; }
        case '6': { cfd_platform_read_key(); return KEY_PGDN; }
        default:  return KEY_ESC;
    }
#endif
}
