#ifndef IPC_H
#define IPC_H

#include "wm.h"

#define SOCKET_PATH "/tmp/mwm.sock"

int ipc_init(void);
void ipc_handle_connection(WMState* state, int listen_fd);
void ipc_cleanup(int listen_fd);

#endif