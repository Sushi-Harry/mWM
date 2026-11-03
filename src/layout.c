#include "../include/layout.h"
#include <X11/Xlib.h>

// void layout_apply_tiling_algo(WMState *state){
//     if(!state->_CLIENTS) return;

//     // Counting the total number of windows first
//     int count = 0;
//     struct Client* curr = state->_CLIENTS;
//     while(curr){
//         count++;
//         curr = curr->_NEXT;
//     }

//     // Defining the master and stack areas
//     // This function is following a Master And Stack appraoch
//     /*
//         Here's what happens:
//             When there's just 1 window, it's the master window
//             2nd window is opened, both get each half of the screen (Left half and right half). Left half is the master window and the right side is the first window in the Stack
//             3rd window is opened, left window remains the master and the right window gets halved and the right half is then shared with the previous and new windows. Both of them are a part of the "Stack" windows
//             same thing for the 4th window. The right half of the screen is divided vertically among the newly opened wnidows
//     */
//     int master_w = (count == 1) ? state->_SCREEN_WIDTH : (state->_SCREEN_WIDTH / 2);
//     int stack_w = state->_SCREEN_WIDTH - master_w;
//     int stack_h = (count > 1) ? (state->_SCREEN_HEIGHT / (count - 1)) : 0;

//     // Looping through all the windows now and then applying their new geometries
//     curr = state->_CLIENTS; int i = 0;
//     while(curr){
//         if(i == 0){
//             curr->_X = 0; curr->_Y = 0;
//             curr->_W = master_w; curr->_H = state->_SCREEN_HEIGHT;
//         }else{
//             curr->_X = master_w; curr->_Y = (i - 1) * stack_h;
//             curr->_W = stack_w; curr->_H = stack_h;
//         }
//         // Applying the calculated geometry now
//         // - 4 cause I'm accounting for 2px width borders
//         XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, curr->_X, curr->_Y, curr->_W - (2 * state->_BORDER), curr->_H - (2 * state->_BORDER));
//         curr = curr->_NEXT;
//         i++;
//     }
// }

void layout_apply_tiling_algo(WMState *state){
    // only counting the windows in the current workspace
    int count = 0;
    for(Client* curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG == state->_CURRENT_TAG)
            count++;
    }

    int master_w = (count == 1) ? state->_SCREEN_WIDTH : (state->_SCREEN_WIDTH / 2);
    int stack_w = state->_SCREEN_WIDTH - master_w;
    int stack_h = (count > 1) ? (state->_SCREEN_HEIGHT / (count - 1)) : 0;

    int i = 0;
    for(Client* curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG != state->_CURRENT_TAG){
            // Now this is a simple trick. The windows in other workspaces are just drawn on the part of the screen that the user cannot see. (Which is outside the rendering resolution)
            XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, state->_SCREEN_WIDTH * 2, 0, curr->_W, curr->_H);
            continue;
        }
        // REST OF THE FUNCTION IS THE SAME AS BEFORE
        if(i == 0){
            curr->_X = 0; curr->_Y = 0;
            curr->_W = master_w; curr->_H = state->_SCREEN_HEIGHT;
        }else{
            curr->_X = master_w; curr->_Y = (i - 1) * stack_h;
            curr->_W = stack_w; curr->_H = stack_h;
        }
        XMoveResizeWindow(state->_DPY, curr->_WINDOW_ID, curr->_X, curr->_Y, curr->_W - (2 * state->_BORDER), curr->_H - (2 * state->_BORDER));
        i++;
    }
}