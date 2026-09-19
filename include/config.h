// dwm style config.h approach
#ifndef CONFIG_H
#define CONFIG_H

#include "core.h"
#include <X11/keysym.h>

static const KeyBinding default_keys[] = {
    { Mod1Mask | ShiftMask, XK_Return, ACTION_SPAWN, { ._COMMAND = "xterm" }},
    { Mod1Mask | ShiftMask, XK_c, ACTION_KILL_CLIENT, { ._COMMAND = NULL }},
    { Mod1Mask | ShiftMask, XK_q, ACTION_QUIT_WM, { ._COMMAND = NULL }}
};

static const size_t default_keys_count = sizeof(default_keys) / sizeof(default_keys[0]);

#endif