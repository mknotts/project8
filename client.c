#include <stdio.h>	// printf
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <unistd.h> // close
#include <sys/time.h> //timeval
#include <stdbool.h>
#include <pthread.h>
#include "udp.h"
#include "client.h"
#include "rpc.h"
#include <sys/socket.h>
#include <arpa/inet.h>

// initializes the RPC connection to the server
struct rpc_connection RPC_init(int src_port, int dst_port, char dst_addr[]){
    // initialize port
    struct socket recv_socket = init_socket(src_port); 

    // populate socket address using
    struct sockaddr_storage dst; // check if this is initialized correctly
    socklen_t addrlen;
    populate_sockaddr(AF_INET, dst_port, dst_addr, &dst, &addrlen);

    // initialize and set rpc_connection arguments
    struct rpc_connection * rpc = malloc(sizeof(struct rpc_connection));
    rpc->recv_socket = recv_socket;
    rpc->dst_addr = *((struct sockaddr *)(&dst));
    rpc->dst_len = addrlen;
    rpc->seq_number = 0;
    rpc->client_id = rand() % 100;
    return *rpc;
}

// Sleeps the server thread for a few seconds
void RPC_idle(struct rpc_connection *rpc, int time){
    struct message * m = malloc(sizeof(struct message));
    strcpy(m->fxn, "idl");
    m->arg1 = time;
    m->arg2 = -1;
    m->clientID = rpc->client_id;
    m->seqNum = ++rpc->seq_number;
    struct packet_info pi;
    for (int i = 0; i < RETRY_COUNT; i++){
        send_packet(rpc->recv_socket, rpc->dst_addr, rpc->dst_len, (char *) m, sizeof(struct message));
        pi = receive_packet_timeout(rpc->recv_socket, TIMEOUT_TIME);
        if (pi.recv_len != 0){
            return;
        }
    }
    perror("Idle request not received\n");
    return;
}

// gets the value of a key on the server store
int RPC_get(struct rpc_connection *rpc, int key){
    struct message * m = malloc(sizeof(struct message));
    strcpy(m->fxn, "get");
    m->arg1 = key;
    m->arg2 = -1;
    m->clientID = rpc->client_id;
    m->seqNum = ++rpc->seq_number;
    struct packet_info pi;
    for (int i = 0; i < RETRY_COUNT; i++){
        send_packet(rpc->recv_socket, rpc->dst_addr, rpc->dst_len, (char *) m, sizeof(struct message));
        pi = receive_packet_timeout(rpc->recv_socket, TIMEOUT_TIME);
        if (pi.recv_len != 0){
            int ret = atoi(pi.buf);
            return ret;
        }
    }
    perror("Get request not received\n");
    return -1;
}

// sets the value of a key on the server store
int RPC_put(struct rpc_connection *rpc, int key, int value){
    struct message * m = malloc(sizeof(struct message));
    strcpy(m->fxn, "put");
    m->arg1 = key;
    m->arg2 = value;
    m->clientID = rpc->client_id;
    m->seqNum = ++rpc->seq_number;
    struct packet_info pi;
    for (int i = 0; i < RETRY_COUNT; i++){
        send_packet(rpc->recv_socket, rpc->dst_addr, rpc->dst_len, (char *) m, sizeof(struct message));
        pi = receive_packet_timeout(rpc->recv_socket, TIMEOUT_TIME);
        if (pi.recv_len != 0){
            int ret = atoi(pi.buf);
            return ret;
        }
    }
    perror("Put request not received\n");
    return -1;
}

// closes the RPC connection to the server
void RPC_close(struct rpc_connection *rpc){
    close_socket(rpc->recv_socket);
    //free(rpc->recv_socket);
    //free(rpc->dst_addr);
    //free(rpc);
}