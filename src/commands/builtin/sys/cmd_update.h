#ifndef CMD_UPDATE_H
#define CMD_UPDATE_H

#include "../../../core/session.h"
#include "../../command.h"

extern const cfd_command_t builtin_update;
int cmd_update(cfd_session_t *sess, int argc, char **argv);

/*
 * Non-blocking startup update check.
 * Reads cached result; if stale, forks a background fetch.
 * If a newer version was found, prints a one-line notice and returns 1.
 * Returns 0 if up-to-date or check unavailable.
 */
int cfd_update_notify_startup(void);

#endif /* CMD_UPDATE_H */
