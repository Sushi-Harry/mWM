#include "../include/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../include/layout.h"
#include "../include/core.h"

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

void ipc_handle_connection(WMState *state, int listen_fd) {
    int client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd < 0) return;

    char buffer[256] = {0};
    ssize_t bytes = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        if (strncmp(buffer, "SET_MASTER_FACT ", 16) == 0) {
            float fact = atof(buffer + 16);
            if (fact >= 0.15F && fact <= 0.90F) {
                state->_MASTER_RATIO = fact;
                layout_apply_tiling_algo(state);
            }
        } 
        else if (strncmp(buffer, "SET_BORDER_WIDTH ", 17) == 0) {
            int bw = atoi(buffer + 17);
            if (bw >= 0 && bw <= 12) {
                state->_BORDER = (uint32_t)bw;
                for (Client *c = state->_CLIENTS; c; c = c->_NEXT) {
                    XSetWindowBorderWidth(state->_DPY, c->_WINDOW_ID, state->_BORDER);
                }
                layout_apply_tiling_algo(state);
            }
        } 
        else if (strncmp(buffer, "SET_BORDER_ACTIVE ", 18) == 0) {
            state->_BORDER_ACTIVE = strtoul(buffer + 18, NULL, 16);
            if (state->_FOCUSED) {
                XSetWindowBorder(state->_DPY, state->_FOCUSED->_WINDOW_ID, state->_BORDER_ACTIVE);
            }
        } 
        else if (strncmp(buffer, "SET_BORDER_INACTIVE ", 20) == 0) {
            state->_BORDER_INACTIVE = strtoul(buffer + 20, NULL, 16);
            for (Client *c = state->_CLIENTS; c; c = c->_NEXT) {
                if (c != state->_FOCUSED) {
                    XSetWindowBorder(state->_DPY, c->_WINDOW_ID, state->_BORDER_INACTIVE);
                }
            }
        } 
        else if (strncmp(buffer, "TOGGLE_BAR", 10) == 0) {
            state->_SHOW_BAR = !state->_SHOW_BAR;
            if (state->_SHOW_BAR) {
                XMapRaised(state->_DPY, state->_BAR);
                update_bar(state);
            } else {
                XUnmapWindow(state->_DPY, state->_BAR);
            }
            layout_apply_tiling_algo(state);
        } 
        else if (strncmp(buffer, "RESTART_WM", 10) == 0) {
            state->_RESTART = true;
            state->_RUNNING = false;
        }

        XFlush(state->_DPY);
    }
    close(client_fd);
}

void ipc_cleanup(int listen_fd) {
    if (listen_fd >= 0) {
        close(listen_fd);
        unlink(SOCKET_PATH);
    }
}