#include "../include/layout.h"
#include <X11/Xlib.h>
#include "../include/config.h"

void layout_apply_tiling_algo(WMState *state){
    // only counting the windows in the current workspace
    int count = 0;
    for(Client* curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG == state->_CURRENT_TAG && !curr->_FLOATING)
            count++;
    }

    int avail_y = BAR_HEIGHT;
    int avail_h = state->_SCREEN_HEIGHT - BAR_HEIGHT;

    int master_w = (count == 1) ? state->_SCREEN_WIDTH : (state->_SCREEN_WIDTH * state->_MASTER_RATIO);
    int stack_w = state->_SCREEN_WIDTH - master_w;
    int stack_h = (count > 1) ? (avail_h / (count - 1)) : 0;

    int i = 0;
    for(Client* curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG != state->_CURRENT_TAG){
            // Now this is a simple trick. The windows in other workspaces are just drawn on the part of the screen that the user cannot see. (Which is outside the rendering resolution)
            XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, state->_SCREEN_WIDTH * 2, 0, curr->_W, curr->_H);
            continue;
        }
        // This handles the floating windows and skips the tiling algo from running on them
        if(curr->_FLOATING){
            XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, curr->_X, curr->_Y, curr->_W - (2 * state->_BORDER), curr->_H - (2 * state->_BORDER));
            continue;
        }
        // REST OF THE FUNCTION IS THE SAME AS BEFORE
        if(i == 0){
            curr->_X = 0; curr->_Y = avail_y;
            curr->_W = master_w; curr->_H = avail_h;
        }else{
            curr->_X = master_w; curr->_Y = avail_y + ((i - 1) * stack_h);
            curr->_W = stack_w; curr->_H = stack_h;
        }
        XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, curr->_X, curr->_Y, curr->_W - (2 * state->_BORDER), curr->_H - (2 * state->_BORDER));
        i++;
    }
}