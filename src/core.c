#include "../include/core.h"
#include "../include/layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xproto.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include "../include/config.h"
#include <X11/Xatom.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

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

static int x_io_error_handler(Display *dpy){
    (void)dpy;
    fprintf(stderr, "[mWM] Fatal: X Server connection lost (Xephyr closed).\n");
    exit(EXIT_SUCCESS);
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
    state->_WM_PROTOCOLS = XInternAtom(state->_DPY, "WM_PROTOCOLS", False);
    state->_WM_DELETE_WINDOW = XInternAtom(state->_DPY, "WM_DELETE_WINDOW", False);
    state->_SCREEN_WIDTH = DisplayWidth(state->_DPY, DefaultScreen(state->_DPY));
    state->_SCREEN_HEIGHT = DisplayHeight(state->_DPY, DefaultScreen(state->_DPY));
    state->_RUNNING = true;
    state->_MASTER_RATIO = 0.5F;
    state->_CLIENTS = NULL;
    state->_FOCUSED = NULL;
    state->_CURRENT_TAG = 1;

    // Set the error handler to the function I made earlier
    XSetErrorHandler(x_error_handler);
    XSetIOErrorHandler(x_io_error_handler);

    // Telling X that windows will be managed by yours truly
    XSelectInput(state->_DPY, state->_ROOT, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(state->_DPY, False);

    // Print into the console that the window and stuff was initialized successfully
    printf("[mWM] Initialized on %s (%d, %d)\n", XDisplayName(NULL), state->_SCREEN_WIDTH, state->_SCREEN_HEIGHT);

    // Creating the taskbar
    state->_BAR = XCreateSimpleWindow(state->_DPY, state->_ROOT, 0, 0, state->_SCREEN_WIDTH, state->_SCREEN_HEIGHT, 0, BlackPixel(state->_DPY, DefaultScreen(state->_DPY)), border_inactive);
    state->_GC = XCreateGC(state->_DPY, state->_BAR, 0, NULL);
    XSetForeground(state->_DPY, state->_GC, WhitePixel(state->_DPY, XDefaultScreen(state->_DPY)));
    XMapRaised(state->_DPY, state->_BAR);
    update_bar(state);

    // Grabkey call
    grab_key(state, keys, KEYS_SIZE);
}
void event_loop_core(WMState *state){
    XEvent xev;
    int x11_fd = ConnectionNumber(state->_DPY);

    Client *drag_client = NULL;
    int drag_button = 0;
    int start_x = 0, start_y = 0, start_wx = 0, start_wy = 0, start_ww = 0, start_wh = 0;

    while(state->_RUNNING){
        while (XPending(state->_DPY)) {
            XNextEvent(state->_DPY, &xev);
        
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

                    XSelectInput(state->_DPY, mev->window, PropertyChangeMask | EnterWindowMask);

                    Client *client = malloc(sizeof(Client));
                    client->_WINDOW_ID = mev->window;
                    client->_X = 0; client->_Y = 0; client->_W = 400; client->_H = 300;
                    client->_FLOATING = false;
                    client->_NEXT = state->_CLIENTS;
                    client->_TAG = state->_CURRENT_TAG;
                    state->_CLIENTS = client;
                    focus_client(state, client);

                    // Calling in the tiling function
                    focus_client(state, client);
                    layout_apply_tiling_algo(state);

                    break;
                }
                case DestroyNotify:
                case UnmapNotify:{
                    Window w = (xev.type == UnmapNotify) ? xev.xunmap.window : xev.xdestroywindow.window;
                    Client **pp = &state->_CLIENTS;
                    bool found = false;
                    while(*pp){
                        Client *curr = *pp;
                        if(curr->_WINDOW_ID == w){
                            *pp = curr->_NEXT;
                            if(state->_FOCUSED == curr){
                                state->_FOCUSED = NULL;
                            }
                            free(curr);
                            found = true;
                            break;
                        }
                        pp = &curr->_NEXT;
                    }
                    if(found){
                        layout_apply_tiling_algo(state);
                        focus_client(state, state->_CLIENTS);
                    }
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
                case PropertyNotify:{
                    XPropertyEvent *ev = &xev.xproperty;
                    if(ev->atom == XA_WM_NAME && state->_FOCUSED && ev->window == state->_FOCUSED->_WINDOW_ID){
                        update_bar(state);
                    }
                    break;
                }
                case ButtonPress: {
                    XButtonEvent *ev = &xev.xbutton;
                    if(ev->subwindow == None) break; // this checks if the background is clicked

                    Client *c = NULL;
                    for(Client *curr = state->_CLIENTS; curr; curr = curr->_NEXT){
                        if(curr->_WINDOW_ID == ev->subwindow) { c = curr; break; }
                    }
                    if(!c) break;

                    focus_client(state, c);

                    if(!c->_FLOATING){
                        c->_FLOATING = true;
                        layout_apply_tiling_algo(state);
                    }

                    drag_client = c;
                    drag_button = ev->button;
                    start_x = ev->x_root;
                    start_y = ev->y_root;
                    start_wx = c->_X; start_wy = c->_Y;
                    start_ww = c->_W; start_wh = c->_H;

                    XGrabPointer(state->_DPY, ev->subwindow, True, PointerMotionMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
                    break;
                }
                case MotionNotify:{
                    if(!drag_client) break;

                    XMotionEvent *ev = &xev.xmotion;
                    int dx = ev->x_root - start_x;
                    int dy = ev->y_root - start_y;
                    if(drag_button == Button1){
                        drag_client->_X = start_wx + dx;
                        drag_client->_Y = start_wy + dy;
                    }else if(drag_button == Button3){
                        drag_client->_W = (start_ww + dx > 50) ? start_ww + dx : 50;
                        drag_client->_H = (start_wh + dy > 50) ? start_wy + dy : 50;
                    }

                    XMoveResizeWindow(state->_DPY, drag_client->_WINDOW_ID, drag_client->_X, drag_client->_Y, drag_client->_W - (2 * state->_BORDER), drag_client->_H - (2 * state->_BORDER));
                    break;
                }
                case ButtonRelease:{
                    if(drag_client){
                        XUngrabPointer(state->_DPY, CurrentTime);
                        drag_client = NULL;
                    }
                    break;
                }
                case EnterNotify:{
                    XEnterWindowEvent *ev = &xev.xcrossing;

                    if(ev->mode != NotifyNormal || ev->detail == NotifyInferior) break;

                    Client *c = NULL;
                    for(Client *curr = state->_CLIENTS; curr; curr = curr->_NEXT){
                        if(curr->_WINDOW_ID == ev->window){
                            c = curr;
                            break;
                        }
                    }
                    if(c && state->_FOCUSED != c){
                        focus_client(state, c);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        fd_set in_fds;
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        struct timeval tval;
        tval.tv_sec = 1;
        tval.tv_usec = 0;

        int num_ready = select(x11_fd + 1, &in_fds, NULL, NULL, &tval);
        if(num_ready == 0){
            update_bar(state);
        }else if(num_ready < 0){
            perror("[mWM] select() error");
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
// This is for the taskbar at the top. Basically, the taskbar is just a window made using X11 with custom text being displayed on it
void update_bar(WMState *state){
    if(!state->_BAR) return;

    XClearWindow(state->_DPY, state->_BAR);

    // This is the string that needs to be displayed on the bar
    char text_left[512];
    char *window_name = NULL;

    if(state->_FOCUSED){
        XFetchName(state->_DPY, state->_FOCUSED->_WINDOW_ID, &window_name);
    }
    snprintf(text_left, sizeof(text_left), "Workspace %d | %s" , state->_CURRENT_TAG, window_name ? window_name : (state->_FOCUSED ? "Unknown" : ""));

    if(window_name){
        XFree(window_name);
    }
    
    // This part is for drawing the time
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[64];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%a, %b %d  %H:%M", timeinfo);

    // drawing the left side of the bar. Workspace nd the title
    XDrawString(state->_DPY, state->_BAR, state->_GC, 10, (BAR_HEIGHT / 2) + 4, text_left, strlen(text_left));
    // drawing the time on the right side
    int time_x = state->_SCREEN_WIDTH - 150;
    XDrawString(state->_DPY, state->_BAR, state->_GC, time_x, (BAR_HEIGHT / 2) + 4, time_str, strlen(time_str));

    XFlush(state->_DPY);
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
            XGrabKey(state->_DPY, code, keys[i]._MOD | modifiers[j], state->_ROOT, False, GrabModeAsync, GrabModeAsync);

        }
    }

    for(size_t i = 0; i < sizeof(modifiers) / sizeof(modifiers[0]); i++){
        // this is for Alt + left click for moving
        XGrabButton(state->_DPY, Button1, MOD_KEY | modifiers[i], state->_ROOT, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
        // this if for Alt + right click for resizing
        XGrabButton(state->_DPY, Button3, MOD_KEY | modifiers[i], state->_ROOT, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    }
}

void focus_client(WMState *state, Client *client){
    if(!client){
        // This runs when there's no such client as the one passed to the function
        XSetInputFocus(state->_DPY, state->_ROOT, RevertToPointerRoot, CurrentTime);
        state->_FOCUSED = NULL;
        update_bar(state);
        return;
    }
    state->_FOCUSED = client;
    // updating the border colors
    for(Client *curr = state->_CLIENTS; curr != NULL; curr = curr->_NEXT){
        if(curr == client)
            XSetWindowBorder(state->_DPY, curr->_WINDOW_ID, border_active);
        else
            XSetWindowBorder(state->_DPY, curr->_WINDOW_ID, border_inactive);
    }

    // Setting the focus to the client
    XSetInputFocus(state->_DPY, client->_WINDOW_ID, RevertToPointerRoot, CurrentTime);
    // Raise the window to the top of the stacking order
    XRaiseWindow(state->_DPY, client->_WINDOW_ID);

    Window root_ret, child_ret;
    int rx, ry, wx, wy;
    unsigned int mask;

    if(XQueryPointer(state->_DPY, client->_WINDOW_ID, &root_ret, &child_ret, &rx, &ry, &wx, &wy, &mask)){
        if(wx < 0 || wy < 0 || wx > client->_W || wy > client->_H){
            XWarpPointer(state->_DPY, None, client->_WINDOW_ID, 0, 0, 0, 0, client->_W / 2, client->_H / 2);
        }
    }

    update_bar(state);    
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
    if(!state->_FOCUSED) return;

    Window w = state->_FOCUSED->_WINDOW_ID;
    int n;
    Atom *protocols = NULL;
    bool supports_delete = false;

    if(XGetWMProtocols(state->_DPY, w, &protocols, &n)){
        for(int i = 0; i < n; i++){
            if(protocols[i] == state->_WM_DELETE_WINDOW){
                supports_delete = true;
                break;
            }
        }
        XFree(protocols);
    }

    if(supports_delete){
        XEvent ev;
        ev.type = ClientMessage;
        ev.xclient.window = w;
        ev.xclient.message_type = state->_WM_PROTOCOLS;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = state->_WM_DELETE_WINDOW;
        ev.xclient.data.l[1] = CurrentTime;

        XSendEvent(state->_DPY, w, False, NoEventMask, &ev);
    }else{
        XKillClient(state->_DPY, w);
    }
}

void quit_wm_action(WMState *state, const Arg *arg) {
    (void)arg;
    state->_RUNNING = false;
}

// WWindow focus switching function definitions :)
void switch_next_action(WMState *state, const Arg *arg){
    (void)arg;
    if(!state->_CLIENTS || !state->_FOCUSED) return;

    // if the _NEXT field exists for the focused window put that as the next_target otherwist set it to the head of the clients linked list
    // Client *next_target = state->_FOCUSED->_NEXT ? state->_FOCUSED->_NEXT : state->_CLIENTS;
    Client *next = NULL;
    Client *current_master = NULL;
    for(Client *curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG == state->_CURRENT_TAG){
            current_master = curr;
            break;
        }
    }
    next = state->_FOCUSED->_NEXT && state->_FOCUSED->_NEXT->_TAG == state->_CURRENT_TAG ? state->_FOCUSED->_NEXT : current_master; 
    focus_client(state, next);
}

void switch_prev_action(WMState *state, const Arg *arg){
    (void)arg;
    if(!state->_CLIENTS || !state->_FOCUSED) return;

    Client *target = NULL, *prev_match = NULL, *last_in_workspace = NULL;

    for(Client* curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG ==state->_CURRENT_TAG){
            if(curr == state->_FOCUSED){
                target = prev_match;
            }

            prev_match = curr;
            last_in_workspace = curr;
        }
    }

    if(!target){
        target = last_in_workspace;
    }
    if(target){
        focus_client(state, target);
    }
}

void view_tag_action(WMState *state, const Arg *arg){
    if(arg->i == state->_CURRENT_TAG) return;

    state->_CURRENT_TAG = arg->i;
    layout_apply_tiling_algo(state);

    Client *target = NULL;
    for(Client *curr = state->_CLIENTS; curr; curr = curr->_NEXT){
        if(curr->_TAG == state->_CURRENT_TAG){
            target = curr;
            break;
        }
    }
    focus_client(state, target);
}

void move_tag_action(WMState* state, const Arg *arg){
    if(state->_FOCUSED && arg->i != state->_CURRENT_TAG){
        state->_FOCUSED->_TAG = arg->i;
        layout_apply_tiling_algo(state);

        // view_tag_action(state, &(Arg){.i = state->_CURRENT_TAG});

    }
}

void shrink_master_action(WMState *state, const Arg *arg){
    (void) arg;
    // clamp the lowest shrink value so that it doesn't shrink into nothingness
    if(state->_MASTER_RATIO > 0.15F){
        state->_MASTER_RATIO -= 0.05F;
        layout_apply_tiling_algo(state);
    }
}

void expand_master_action(WMState *state, const Arg *arg){
    (void)arg;
    if(state->_MASTER_RATIO < 0.9F){
        state->_MASTER_RATIO += 0.05F;
        layout_apply_tiling_algo(state);
    }
}

void toggle_floating_action(WMState *state, const Arg *arg){
    (void) arg;
    if(!state->_FOCUSED) return;

    state->_FOCUSED->_FLOATING = !state->_FOCUSED->_FLOATING;
    layout_apply_tiling_algo(state);
}