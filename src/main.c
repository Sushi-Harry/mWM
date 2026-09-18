#include "../include/wm.h"
#include "../include/core.h"

int main(){
    WMState state = {0};

    initialize_core(&state);
    event_loop_core(&state);
    cleanup_core(&state);

    return 0;
}