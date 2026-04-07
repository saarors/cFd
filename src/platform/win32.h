#ifndef CFD_WIN32_H
#define CFD_WIN32_H

#ifdef _WIN32

#include <windows.h>

/* Windows-specific helpers */
void  win32_enable_virtual_terminal(void);
DWORD win32_get_console_mode(HANDLE h);
void  win32_set_console_mode(HANDLE h, DWORD mode);

/* Convert Windows error to string */
const char *win32_strerror(DWORD err);

#endif /* _WIN32 */
#endif /* CFD_WIN32_H */
