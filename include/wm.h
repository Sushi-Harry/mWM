#ifndef WM_H
#define WM_H

#include "client.h"
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum{
    ACTION_SPAWN,
    ACTION_KILL_CLIENT,
    ACTION_QUIT_WM,
    ACTION_FOCUS_NEXT,
    ACTION_FOCUS_PREVIOUS
} WMAction;

typedef struct {
    Display *_DPY;
    Window _ROOT;
    uint32_t _SCREEN_WIDTH, _SCREEN_HEIGHT;
    uint32_t _BORDER;
    unsigned long _BORDER_COLOR;
    Client *_CLIENTS;
    Client *_FOCUSED;
    bool _RUNNING;
    uint32_t _CURRENT_TAG;

    Window _BAR;
    GC _GC;

    float _MASTER_RATIO;

    Atom _WM_PROTOCOLS;
    Atom _WM_DELETE_WINDOW;

    // EWMH Atoms
    Atom _NET_SUPPORTED;
    Atom _NET_CLIENT_LIST;
    Atom _NET_NUMBER_OF_DESKTOPS;
    Atom _NET_CURRENT_DESKTOP;
    Atom _NET_ACTIVE_WINDOW;
    Atom _NET_WM_NAME;
    Atom _NET_SUPPORTING_WM_CHECK;
    Atom _UTF8_STRING;

} WMState;

#endif