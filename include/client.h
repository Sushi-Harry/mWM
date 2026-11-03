#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct Client{
    int _WINDOW_ID;
    bool _FLOATING;
    unsigned int _W, _H, _X, _Y;
    unsigned int _TAG;
    struct Client* _NEXT;
}Client;

#endif