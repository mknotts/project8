#include <stdio.h>	// printf
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <unistd.h> // close
#include <sys/time.h> //timeval
#include <pthread.h>
#include "rpc.h"
#include "server_functions.h"

struct client{
    int clientID;
    int seqNum;
    int lastRes;
};

struct socket s;

void * call_function(void * arg){
    struct packet_info pi = *((struct packet_info *) arg);
	struct message * m = (struct message *) (pi.buf);
    char * ret = malloc(sizeof(int));

    if (strcmp(m->fxn, "idl") == 0){
        idle(m->arg1);
        sprintf(ret, "%d", 0);
    } else if (strcmp(m->fxn, "put") == 0){
        int res = put(m->arg1, m->arg2);
        sprintf(ret, "%d", res);
    } else if (strcmp(m->fxn, "get") == 0){
        int res = get(m->arg1);
        sprintf(ret, "%d", res);
    }
    send_packet(s, pi.sock, pi.recv_len, ret, sizeof(int));
    void * r = malloc(sizeof(1));
    return r;
}

int main(){
    s = init_socket(SERVER_PORT);
    while (1){
        struct packet_info pi = receive_packet(s);
        if (pi.recv_len != 0){
            pthread_t * child_thread = malloc(sizeof(child_thread));
            pthread_create(child_thread, NULL, call_function, &pi);
        }
    }
}