#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h> 
#include <sys/stat.h>  
#include <sys/ipc.h>
#define max_msg_len 1000
#define max_clients 10
#define SERVER_QUEUE "/queue_name_server"
#define max_wait_for_init 1000000
#define queue_name_len 8
#define max_id_len 2

typedef enum msg_type{
    STOP = 6,
    DISCONNECT = 5,
    LIST = 4,
    CONNECT = 3,
    INIT = 2,
    MSG = 1
}msg_type; 

typedef struct client{
    mqd_t queue_id;
    char queue_name[10];
    int client_id;
    int interlocutor; //id of client's current interlocutor. If disconnected should be -2 if never connected -1
}client;


int parse_str_to_type(char *str);
void set_sigint_handling(void (*func)(int,  siginfo_t *, void *));
void exit_error(char *content);
void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext);
mqd_t create_queue(int maxmsg, int no_block, char* name);


#endif