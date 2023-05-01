#include <stdio.h>	// printf
#include <string.h> // memset
#include <stdlib.h> // exit(0);
#include <unistd.h> // close
#include <sys/time.h> //timeval
#include <client.h>
#include <socket.h>
#include <message.h>

// initializes the RPC connection to the server
struct rpc_connection RPC_init(int src_port, int dst_port, char dst_addr[]){
    // initialize port
    struct socket recv_socket = init_socket(src_port); 

    // populate socket address using
    struct socketaddr_storage dst = malloc(sizeof(socketaddr_storage)); // is malloc necessary?
    socklen_t addrlen = 0;
    populate_sockaddr(AF_INET, dst_port, dst_addr, dst, addrlen);

    // initialize and set rpc_connection arguments
    struct rpc_connection rpc = malloc(sizeof(rpc_connection));
    rpc.recv_socket = recv_socket;
    rpc.dst_addr = *((struct sockaddr *)(&addr));
    rpc.dst_len = addrlen;
    rpc.seq_number = 0;
    rpc.client_id = rand();
    return rpc;
}

// Sleeps the server thread for a few seconds
void RPC_idle(struct rpc_connection *rpc, int time){
    struct message m = malloc(sizeof(message));
    m.fxn = "idl";
    m.arg1 = time;
    m.arg2 = -1;
    // send_packet(struct socket source, struct sockaddr target, int slen, char* payload, int payload_length);
    struct packet_info pi;
    for (int i = 0; i < RETRY_COUNT; i++){
        send_packet(rpc->recv_socket, rpc->dst_addr, rpc->dst_len, (char *) m, sizeof(message));
        pi = receive_packet_timeout(TIMEOUT_TIME);
        if (strlen((char*)packet_info) != 0){
            return 0;
        }
    }
    perror("Idle request not received\n");
    return 1;
}

// gets the value of a key on the server store
int RPC_get(struct rpc_connection *rpc, int key){

}

// sets the value of a key on the server store
int RPC_put(struct rpc_connection *rpc, int key, int value){

}

// closes the RPC connection to the server
void RPC_close(struct rpc_connection *rpc){

}