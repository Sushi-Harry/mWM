#ifndef CORE_H
#define CORE_H
#include "wm.h"

struct Key;
// Function to initialize the core
void initialize_core(WMState* state);
// This one is for the event loop
void event_loop_core(WMState* state);
// This is the cleanup function
void cleanup_core(WMState* state);

// This is a helper function to spawn a window
void spawn(const char *command);
void focus_client(WMState *state, Client *client);

// Key grabbing
void grab_key(WMState* state, const struct Key* keys, size_t keys_count);

// This one updates the top bar
void update_bar(WMState *state);

#endif