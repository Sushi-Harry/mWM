#include "../include/core.h"
#include "../include/layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xproto.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include "../include/config.h"

// This function handles the errors in case a window misbehaves so that the windowmanager doesn't instantly crash
static int x_error_handler(Display *dpy, XErrorEvent *ee){
    // This is some peak window handling right here. Takee notes kids
    if (ee->error_code == BadWindow ||
        (ee->request_code == X_ConfigureWindow && ee->error_code == BadWindow)){
        return 0;
    }

      // the XErrorEvent variable stores info for the error that was caught
      fprintf(stderr, "[mWM] X11 Error: Request Code %d, Error Code %d\n",
              ee->request_code, ee->error_code);
    return 0;
}

void initialize_core(WMState *state){
    // Basically initializing the variables for the WMState *state variable
    state->_DPY = XOpenDisplay(NULL);

    // Checking if the display variable (state->_DPY) was initialized or is it null
    if(!state->_DPY){
        fprintf(stderr, "FATAL: Could Not Open X Display %s\n", XDisplayName(NULL));
        exit(EXIT_FAILURE);
    }
    /// Past this point the program runs if the display was initialized without issues
    state->_ROOT = DefaultRootWindow(state->_DPY);
    state->_SCREEN_WIDTH = DisplayWidth(state->_DPY, DefaultScreen(state->_DPY));
    state->_SCREEN_HEIGHT = DisplayHeight(state->_DPY, DefaultScreen(state->_DPY));
    state->_RUNNING = true;

    // Set the error handler to the function I made earlier
    XSetErrorHandler(x_error_handler);

    // Telling X that windows will be managed by yours truly
    XSelectInput(state->_DPY, state->_ROOT, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(state->_DPY, False);

    // Print into the console that the window and stuff was initialized successfully
    printf("[mWM] Initialized on %s (%d, %d)\n", XDisplayName(NULL), state->_SCREEN_WIDTH, state->_SCREEN_HEIGHT);

    // Grabkey call
    grab_key(state, keys, KEYS_SIZE);
}

void event_loop_core(WMState *state){
    XEvent xev;
    while(state->_RUNNING && !XNextEvent(state->_DPY, &xev)){
        // Event handling will be done here
        switch (xev.type) {
            // This is the window configure request event being handled
            case ConfigureRequest:{
                XConfigureRequestEvent *ev = &xev.xconfigurerequest;
                XWindowChanges wc;
                wc.x = ev->x;
                wc.y = ev->y;
                wc.width = ev->width;
                wc.height = ev->height;
                wc.border_width = ev->border_width;
                wc.sibling = ev->above;
                wc.stack_mode = ev->detail;

                // Applying the changes tot he xserver
                XConfigureWindow(state->_DPY, ev->window, ev->value_mask, &wc);
                break;
            }
            case MapRequest:{
                XMapRequestEvent *mev = &xev.xmaprequest;
                // This is for adding a border so that the windows are visible
                XSetWindowBorderWidth(state->_DPY, mev->window, state->_BORDER);
                XSetWindowBorder(state->_DPY, mev->window, state->_BORDER_COLOR); // that last argument is just the color code in hex
                // Now mapping the actual window
                XMapWindow(state->_DPY, mev->window);

                Client *client = malloc(sizeof(Client));
                client->_WINDOW_ID = mev->window;
                client->_X = 0; client->_Y = 0; client->_W = 400; client->_H = 300;
                client->_FLOATING = false;
                client->_NEXT = state->_CLIENTS;
                state->_CLIENTS = client;

                // Calling in the tiling function
                layout_apply_tiling_algo(state);

                break;
            }
            case DestroyNotify:
            case UnmapNotify:{
                Window w = (xev.type == UnmapNotify) ? xev.xunmap.window : xev.xdestroywindow.window;
                Client **pp = &state->_CLIENTS;
                while(*pp){
                    Client *curr = *pp;
                    if(curr->_WINDOW_ID == w){
                        *pp = curr->_NEXT;
                        free(curr);
                        break;
                    }
                    pp = &curr->_NEXT;
                }
                layout_apply_tiling_algo(state);
                break;
            }
            case KeyPress:{
                XKeyEvent *kev = &xev.xkey;
                KeySym keysym = XLookupKeysym(kev, 0);
                
                unsigned int clean_state = kev->state & ~(LockMask | Mod2Mask);
                
                for(size_t i = 0; i < KEYS_SIZE; i++){
                    if(keysym == keys[i]._KEYSYM && clean_state == keys[i]._MOD && keys[i].func){
                        keys[i].func(state, &keys[i]._ARG);
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

void cleanup_core(WMState *state){
    if(state->_DPY){
        XCloseDisplay(state->_DPY);
    }
    printf("[mWM] Shutting Down\n");
}

void spawn(const char *command){
    if(fork() == 0){
        if(fork() == 0){
            // Child process execute command
            char *args[] = {(char*)command, NULL};
            execvp(command, args);
            perror("[mWM execvp failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    wait(NULL);
}

// Here's some explanation
/*
    So X11 uses KeyCodes that are hardware-dependent integers instead of using raw ASCII.
    So to pass input data to XGrabKey, we need to convert normal ASCII, or rather KeySyms to KeyCodes that X11 can understand
    For that we'll use the XKeySymToKeycode() function and then register the input with XGrabKey()
*/

void grab_key(WMState* state, const Key* keys, size_t keys_count){
    // ungrab any existing keys
    XUngrabKey(state->_DPY, AnyKey, AnyModifier, state->_ROOT);
    // This part is to ignore modifier keys such as Caps Lock, num lock, scroll lock etc.
    unsigned int modifiers[] = { 0, LockMask, Mod2Mask, LockMask | Mod2Mask };

    for(size_t i = 0; i < keys_count ; i++){
        KeyCode code = XKeysymToKeycode(state->_DPY, keys[i]._KEYSYM);
        if(code == 0) continue;
        for(size_t j = 0; j < sizeof(modifiers) / sizeof(modifiers[0]); j++){
            XGrabKey(state->_DPY, code, keys[i]._MOD | modifiers[j], state->_ROOT, True, GrabModeAsync, GrabModeAsync);

        }
    }
}

// These are the definitions of the functions in config.h that are called when an action is supposed to take place
void spawn_action(WMState *state, const Arg *arg){
    (void)state;
    if(arg->_COMMAND){
        spawn(arg->_COMMAND);
    }
}

void kill_action(WMState *state, const Arg *arg){
    (void)arg;
    if(state->_CLIENTS){
        XKillClient(state->_DPY, state->_CLIENTS->_WINDOW_ID);
    }
}

void quit_wm_action(WMState *state, const Arg *arg) {
    (void)arg;
    state->_RUNNING = false;
}