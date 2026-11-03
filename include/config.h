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
// Window focus switching function signatures blah blah blah blah blah AAAAAAAAAAAAAAAAAAAAAAAAAAA
void switch_next_action(WMState *state, const Arg *arg);
void switch_prev_action(WMState *state, const Arg *arg);
void view_tag_action(WMState *state, const Arg *arg);
void move_tag_action(WMState *state, const Arg *arg);

// This is an array for basic keybinds. The user will have to edit this config and recompile the project everytime if they wanna edit the keybinds
// There's advantages to this but yeah I do understand that this isn't really a user friendly approach. But it's my project. I get to decide what philosophy I wanna follow
static const Key keys[] = {
    { MOD_KEY | ShiftMask, XK_Return, spawn_action, { ._COMMAND="xterm"}},
    { MOD_KEY, XK_e, spawn_action, { ._COMMAND="thunar" }},
    { MOD_KEY, XK_Right, switch_next_action, { ._COMMAND=NULL}},
    { MOD_KEY, XK_Left, switch_prev_action, { ._COMMAND=NULL}},
    { MOD_KEY | ShiftMask, XK_c, kill_action, { ._COMMAND=NULL }},
    { MOD_KEY | ShiftMask, XK_q, quit_wm_action, { ._COMMAND=NULL }},
    // WORKSPACE NAVIGATION STUFF
    { MOD_KEY, XK_1, view_tag_action, {.i = 1 }},
    { MOD_KEY | ShiftMask, XK_1, move_tag_action, { .i = 1 }},
    { MOD_KEY, XK_2, view_tag_action, {.i = 2 }},
    { MOD_KEY | ShiftMask, XK_2, move_tag_action, { .i = 2 }},
    { MOD_KEY, XK_3, view_tag_action, {.i = 3 }},
    { MOD_KEY | ShiftMask, XK_3, move_tag_action, { .i = 3 }},
    { MOD_KEY, XK_4, view_tag_action, {.i = 4 }},
    { MOD_KEY | ShiftMask, XK_4, move_tag_action, { .i = 4 }},
    { MOD_KEY, XK_5, view_tag_action, {.i = 5 }},
    { MOD_KEY | ShiftMask, XK_5, move_tag_action, { .i = 5 }},
    { MOD_KEY, XK_6, view_tag_action, {.i = 6 }},
    { MOD_KEY | ShiftMask, XK_6, move_tag_action, { .i = 6 }},
    { MOD_KEY, XK_7, view_tag_action, {.i = 7 }},
    { MOD_KEY | ShiftMask, XK_7, move_tag_action, { .i = 7 }},
    { MOD_KEY, XK_8, view_tag_action, {.i = 8 }},
    { MOD_KEY | ShiftMask, XK_8, move_tag_action, { .i = 8 }},
    { MOD_KEY, XK_9, view_tag_action, {.i = 9 }},
    { MOD_KEY | ShiftMask, XK_9, move_tag_action, { .i = 9 }},
};

// static const size_t default_keys_count = sizeof(default_keys) / sizeof(default_keys[0]);
static const size_t KEYS_SIZE = sizeof(keys) / sizeof(keys[0]);

#endif