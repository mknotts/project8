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

int main(){
    s = init_socket(SERVER_PORT);
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
                printf("incoming seqnum equal to server seqnum\n");
                char * ret = malloc(sizeof(int));
                sprintf(ret, "%d", clients[m->clientID].lastRes);
                // printf("last result: %d\n", clients[m->clientID].lastRes);
                // printf("ret: %s\n", ret);
                send_packet(s, pi.sock, sizeof(struct sockaddr_storage), ret, sizeof(int) + 1);                
                continue;
            } else if (last > i){
                printf("incoming seqnum less than server seqnum\n");
                continue;
            } 
            printf("incoming seqnum greater than server seqnum\n");
        }
        clients[m->clientID].seqNum++;
        if (pi.recv_len != 0){
            pthread_t * child_thread = malloc(sizeof(child_thread));
            pthread_create(child_thread, NULL, call_function, &pi);
            pthread_join(*child_thread, NULL);
        }
    }
}

/**
// Function to initialize the call table
void initialize_call_table() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        call_table[i].client_id = -1;
        call_table[i].last_seq_num = -1;
        call_table[i].last_result = -1;
    }
}


// Function to handle incoming client messages
void handle_client_message(char* message, int message_len, struct sockaddr_in* client_address) {
    // Parse the message to get the client ID and sequence number
    int client_id;
    int seq_num;
    memcpy(&client_id, message, sizeof(int));
    memcpy(&seq_num, message + sizeof(int), sizeof(int));

    // Look up the client in the call table
    client_call_state* call_state = NULL;
    pthread_mutex_lock(&call_table_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (call_table[i].client_id == client_id) {
            call_state = &call_table[i];
            break;
        }
    }

    // If the client is not found in the call table, add them
    if (call_state == NULL) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (call_table_mutex[i].client_id == -1) {
                call_state = &call_table[i];
                call_state->client_id = client_id;
                call_state->last_seq_num = -1;
                call_state->last_result = -1;
                break;
            }
        }
    }
}
**/
/**
int listen_for_packets(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        printf("Error creating socket\n");
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)
    };

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Error binding to port %d\n", port);
        exit(1);
    }

    return sockfd;
}

struct packet* get_next_packet(int sockfd) {
    struct packet* packet = (struct packet*)malloc(sizeof(struct packet));
    memset(packet, 0, sizeof(struct packet));

    socklen_t socklen = sizeof(packet->sock);
    int bytes_received = recvfrom(sockfd, packet->data, MAX_PACKET_SIZE, 0, (struct sockaddr*)&packet->sock, &socklen);
    if (bytes_received == -1) {
        printf("Error receiving packet\n");
        exit(1);
    }

    return packet;
}

struct rpc_connection RPC_init(int port, int client_id, char* ip_addr) {
    struct rpc_connection rpc;
    rpc.port = port;
    rpc.client_id = client_id;
    rpc.ip_addr = ip_addr;
    return rpc;
} {
    int port;
    int client_id;
    char* ip_addr;
};

struct rpc_entry {
    char* key;
    char* value;
};

struct rpc_table {
    int num_entries;
    struct rpc_entry* entries;
};

int listen_for_packets(int port);
struct packet* get_next_packet(int sockfd);
struct rpc_connection RPC_init(int port, int client_id, char* ip_addr);
char* RPC_get(struct rpc_connection* rpc, char* key);
void RPC_put(struct rpc_connection* rpc, char* key, char* value);
struct rpc_table* RPC_get_table(struct rpc_connection* rpc);
void RPC_free_table(struct rpc_table* table);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int sockfd = listen_for_packets(port);

    printf("Server listening on port %d...\n", port);

    while (1) {
        struct packet* packet = get_next_packet(sockfd);
        int client_id = packet->sock.sin_port;
        struct rpc_connection rpc = RPC_init(port, client_id, inet_ntoa(packet->sock.sin_addr));

        char* key = strtok(packet->data, ",");
        char* value = strtok(NULL, ",");
        if (value == NULL) {
            char* result = RPC_get(&rpc, key);
            if (result == NULL) {
                printf("Key not found: %s\n", key);
            } else {
                printf("Got value for key %s: %s\n", key, result);
            }
        } else {
            RPC_put(&rpc, key, value);
            printf("Set value for key %s: %s\n", key, value);
        }

        free(packet);
    }

    return 0;
}

int listen_for_packets(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        printf("Error creating socket\n");
        exit(1);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)
    };

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Error binding to port %d\n", port);
        exit(1);
    }

    return sockfd;
}

struct packet* get_next_packet(int sockfd) {
    struct packet* packet = (struct packet*)malloc(sizeof(struct packet));
    memset(packet, 0, sizeof(struct packet));

    socklen_t socklen = sizeof(packet->sock);
    int bytes_received = recvfrom(sockfd, packet->data, MAX_PACKET_SIZE, 0, (struct sockaddr*)&packet->sock, &socklen);
    if (bytes_received == -1) {
        printf("Error receiving packet\n");
        exit(1);
    }

    return packet;
}

struct rpc_connection RPC_init(int port, int client_id, char* ip_addr) {
    struct rpc_connection rpc;
    rpc.port = port;
    rpc.client_id = client_id;
    rpc.ip_addr = ip_addr;
    return rpc;
}

//char* RPC
**/