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
} WMState;

#endif