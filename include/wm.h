#ifndef WM_H
#define WM_H

#include "client.h"
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Display *_DPY;
    Window _ROOT;
    uint32_t _SCREEN_WIDTH, _SCREEN_HEIGHT;
    uint32_t _BORDER;
    unsigned long _BORDER_COLOR;
    Client *_CLIENTS;
    bool _RUNNING;
} WMState;

#endif