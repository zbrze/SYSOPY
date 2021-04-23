#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#define max_msg_len 1000
#define max_clients 10
#define KEY_PATH getenv("HOME")
#define KEY_GEN 19
#define max_wait_for_init 1000000

typedef enum msg_type{
    STOP = 1,
    DISCONNECT = 2,
    LIST = 3,
    CONNECT = 4,
    INIT = 5,
    MSG = 6
}msg_type; 

typedef struct msg{
    long type;
    char content[max_msg_len];
    int client_id;
    int failed;
}msg;
typedef struct client{
    int queue_id;
    int client_id;
    int interlocutor; //id of client's current interlocutor. If disconnected should be -2 if never connected -1
}client;


int parse_str_to_type(char *str);
char* parse_type_to_str(int type);
void print_msg(msg m);
void set_sigint_handling(void (*func)(int,  siginfo_t *, void *));
void exit_error(char *content);
void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext);
#define max_msg_size sizeof(msg)- sizeof(long)
#endif