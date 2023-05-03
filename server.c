#include <stdio.h>	// printf
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <unistd.h> // close
#include <sys/time.h> //timeval		      
#include <pthread.h>
#include "rpc.h"
#include "server_functions.h"
#include <stdbool.h>
#include <pthread.h>
#include "udp.h"
#include "server_functions.h"
#include <sys/socket.h>
#include <arpa/inet.h>

struct client{
    int clientID;
    int seqNum;
    int lastRes;
};

struct socket s;
struct client clients[100];

void * call_function(void * arg){
    struct packet_info pi = *((struct packet_info *) arg);
	struct message * m = (struct message *) (pi.buf);
    char * ret = malloc(sizeof(int));
    if (strcmp(m->fxn, "idl") == 0){
        idle(m->arg1);
        sprintf(ret, "%d", 0);
        clients[m->clientID].lastRes = 0;
    } else if (strcmp(m->fxn, "put") == 0){
        int res = put(m->arg1, m->arg2);
        sprintf(ret, "%d", res);
        clients[m->clientID].lastRes = res;
    } else if (strcmp(m->fxn, "get") == 0){
        int res = get(m->arg1);
        sprintf(ret, "%d", res);
        clients[m->clientID].lastRes = res;
    }

    send_packet(s, pi.sock, sizeof(struct sockaddr_storage), ret, sizeof(int) + 1);
    void * r = malloc(sizeof(1));
    return r;
}

int main(int argc, char**argv){
    if (argc < 2){
        perror("Didn't enter a port number\n");
        return 0;
    }
    const int server_num = atoi(argv[1]);
    s = init_socket(server_num);
    for (int i = 0; i < 100; i++){
        clients[i].clientID = -1;
    } 
    while (1){
        struct packet_info pi = receive_packet(s);
        struct message * m = (struct message *) (pi.buf);
        // printf("function: %s\n", m->fxn);
        // printf("i: %d\n", m->seqNum);
        // printf("last: %d\n", clients[m->clientID].seqNum);
        if (clients[m->clientID].clientID == -1){
            clients[m->clientID].clientID = m->clientID;
            clients[m->clientID].seqNum = 0;
            clients[m->clientID].lastRes = -1;
        } else {
            int i = m->seqNum;
            int last = clients[m->clientID].seqNum;
            if (last == i){
                //printf("incoming seqnum equal to server seqnum\n");
                char * ret = malloc(sizeof(int));
                sprintf(ret, "%d", clients[m->clientID].lastRes);
                // printf("last result: %d\n", clients[m->clientID].lastRes);
                // printf("ret: %s\n", ret);
                send_packet(s, pi.sock, sizeof(struct sockaddr_storage), ret, sizeof(int) + 1);                
                continue;
            } else if (last > i){
                //printf("incoming seqnum less than server seqnum\n");
                continue;
            } 
            //printf("incoming seqnum greater than server seqnum\n");
        }
        clients[m->clientID].seqNum++;
        if (pi.recv_len != 0){
            pthread_t * child_thread = malloc(sizeof(child_thread));
            pthread_create(child_thread, NULL, call_function, &pi);
            pthread_join(*child_thread, NULL);
        }
    }
}