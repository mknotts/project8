#ifndef RPC_h
#define RPC_h

#include "udp.h"


struct message{
    char fxn[4];
    int arg1;
    int arg2;
};

#define SERVER_PORT 8888


#endif