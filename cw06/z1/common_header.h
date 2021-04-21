#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <stdlib.h>

#define max_msg_len 1000
#define max_clients 10
#define KEY_PATH getenv("HOME")
#define KEY_GEN 19

typedef enum msg_type{
    STOP = 1,
    CONNECT = 2,
    DISCONNECT = 3,
    LIST = 4,
    INIT = 5
}msg_type; 

typedef struct msg{
    long type;
    char content[max_msg_len];
    int client_id;
}msg;

typedef struct client{
    int queue_id;
    int client_id;
    int interlocutor; //id of client's current interlocutor. If disconnected should be -2 if never connected -1
}client;

int parse_to_msg_type(char *str);
char* parse_type_to_str(int type);
void print_msg(msg m);
#define max_msg_size sizeof(msg)- sizeof(long)
#endif