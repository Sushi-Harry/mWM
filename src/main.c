#include "../include/wm.h"
#include "../include/core.h"

int main(){
    WMState state = {0};
    state._BORDER = 10;
    state._BORDER_COLOR = 0xff0000;

    initialize_core(&state);
    event_loop_core(&state);
    cleanup_core(&state);

    return 0;
}