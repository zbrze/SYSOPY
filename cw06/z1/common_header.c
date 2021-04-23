#include<string.h>
#include<stdio.h>
#include"common_header.h"

int parse_str_to_type(char *str){
    if(!strcasecmp(str, "STOP")) return STOP;
    if(!strcasecmp(str, "CONNECT")) return CONNECT;
    if(!strcasecmp(str, "DISCONNECT")) return DISCONNECT;
    if(!strcasecmp(str, "LIST")) return LIST;
    if(!strcasecmp(str, "INIT")) return INIT;
    if(!strcasecmp(str, "MSG")) return MSG;
    return __INT_MAX__;
}

char* parse_type_to_str(int type){
    switch (type)
    {
    case STOP:
        return "STOP";
    case CONNECT:
        return "CONNECT";
    case DISCONNECT:
        return "DISCONNECT";
    case LIST:
        return "LIST";
    case INIT:
        return "INIT";
    case MSG:
        return "MSG";
    default:
        return "UNKNOWN";
    }
}

void print_msg(msg m){
    printf("\n---Message---\n");
    printf("Client id: %d\n", m.client_id);
    printf("Message type: %s\n", parse_type_to_str(m.type));
    printf("Failed: %s\n", (m.failed) ? "yes" : "no");
    printf("Content: %s\n", m.content);
    printf("---------------\n");
}
void set_sigint_handling(void (*handler)(int,  siginfo_t *, void *)){
    struct sigaction action;
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
}
void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("\nreceived sigint, going to stop\n");
    exit(0);
}
void exit_error(char *content){
    perror(content);
    exit(-1);
}
