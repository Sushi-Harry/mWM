#include "../include/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../include/layout.h"

// I actually don't know much about this because this is supposed to be an additional feature. Not the foundation of the basic window manager
// Bad argument but it is what it is
int ipc_init(){
    unlink(SOCKET_PATH);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) return -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        close(fd);
        return -1;
    }

    listen(fd, 5);
    return fd;
}

void ipc_handle_connection(WMState *state, int listen_fd){
    int client_fd = accept(listen_fd, NULL, NULL);
    if(client_fd < 0) return;

    char buffer[256] = {0};
    ssize_t bytes = read(client_fd, buffer, sizeof(buffer) - 1);
    if(bytes > 0){
        if(strncmp(buffer, "SET_MASTER_FACT ", 16) == 0){
            float fact = atof(buffer + 16);
            if(fact >= 0.1F && fact <= 0.9F){
                state->_MASTER_RATIO = fact;
                layout_apply_tiling_algo(state);
            }
        }else if (strncmp(buffer, "TOGGLE_BAR", 10) == 0) {
            // logic to toggle state->_BAR height
        }
    }
    close(client_fd);
}