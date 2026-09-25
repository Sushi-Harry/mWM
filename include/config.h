// dwm style config.h approach
#ifndef CONFIG_H
#define CONFIG_H

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <unistd.h>
#include "wm.h"

#define MOD_KEY Mod1Mask

static const unsigned int border_width = 2;
static const unsigned long border_inactive = 0x444444; 
static const unsigned long border_active = 0x420420; // I have no idea what color this is. But if someone is reading this then you probably know why I have this as the default value.

typedef union{
    const char* _COMMAND;
    int i;
} Arg;

static const char *terminal_arg = "xterm";
static const char *browser_arg = "firefox";
static const char *fileman_arg = "thunar";

typedef struct Key{
    unsigned int _MOD;
    KeySym _KEYSYM;
    void (*func)(WMState *state, const Arg *arg);
    Arg _ARG;
} Key;

// These are the functions that'll handle the actions
void spawn_action(WMState *state, const Arg *arg);
void kill_action(WMState *state, const Arg *arg);
void quit_wm_action(WMState *state, const Arg *arg);

// This is an array for basic keybinds. The user will have to edit this config and recompile the project everytime if they wanna edit the keybinds
// There's advantages to this but yeah I do understand that this isn't really a user friendly approach. But it's my project. I get to decide what philosophy I wanna follow
static const Key keys[] = {
    { MOD_KEY | ShiftMask, XK_Return, spawn_action, { ._COMMAND="xterm"}},
    { MOD_KEY, XK_e, spawn_action, { ._COMMAND="thunar" }},
    { MOD_KEY | ShiftMask, XK_c, kill_action, { ._COMMAND=NULL }},
    { MOD_KEY | ShiftMask, XK_q, quit_wm_action, { ._COMMAND=NULL }},
};

// static const size_t default_keys_count = sizeof(default_keys) / sizeof(default_keys[0]);
static const size_t KEYS_SIZE = sizeof(keys) / sizeof(keys[0]);

#endif