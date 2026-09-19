#ifndef CORE_H
#define CORE_H
#include "wm.h"

typedef enum{
    ACTION_SPAWN,
    ACTION_KILL_CLIENT,
    ACTION_QUIT_WM,
    ACTION_FOCUS_NEXT,
    ACTION_FOCUS_PREVIOUS
} WMAction;
typedef union{
    const char* _COMMAND;
    int i;
} Arg;
// Keybinds struct
typedef struct{
    unsigned int _MOD;
    KeySym _KEYSYM;
    WMAction _ACTION;
    Arg _ARG;
} KeyBinding;


// Function to initialize the core
void initialize_core(WMState* state);
// This one is for the event loop
void event_loop_core(WMState* state);
// This is the cleanup function
void cleanup_core(WMState* state);

// This is a helper function to spawn a window
void spawn(const char *command);

// Key grabbing
void grab_key(WMState* state, const KeyBinding* keys, size_t keys_count);
#endif