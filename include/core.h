#ifndef CORE_H
#define CORE_H
#include "wm.h"

// Function to initialize the core
void initialize_core(WMState* state);
// This one is for the event loop
void event_loop_core(WMState* state);
// This is the cleanup function
void cleanup_core(WMState* state);

// This is a helper function to spawn a window
void spawn(const char *command);

// Key grabbing
void grab_key(WMState *state);
#endif