#ifndef WM_H
#define WM_H

#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Display *_DPY;
    Window _ROOT;
    uint32_t _SCREEN_WIDTH, _SCREEN_HEIGHT;
    bool _RUNNING;
} WMState;

#endif