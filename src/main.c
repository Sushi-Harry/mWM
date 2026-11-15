#include "../include/wm.h"
#include "../include/core.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv){
    (void)argc;
    WMState state = {0};

    initialize_core(&state);
    event_loop_core(&state);
    cleanup_core(&state);

    if(state._RESTART){
        execvp(argv[0], argv);
        perror("[mWM] execvp restart failed");
    }
    return 0;
}