#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct Client{
    int _WINDOW_ID;
    bool _FLOATING;
    unsigned int _W, _H, _X, _Y;
    unsigned int _TAGS; // Have to decide what this could be used for. Could be used to store the workspace that the client window is in right now
    
    struct Client* _NEXT;
}Client;

#endif