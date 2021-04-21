#include<string.h>
#include<stdio.h>
#include"common_header.h"

int parse_to_msg_type(char *str){
    if(!strcasecmp(str, "STOP")) return STOP;
    if(!strcasecmp(str, "CONNECT")) return CONNECT;
    if(!strcasecmp(str, "DISCONNECT")) return DISCONNECT;
    if(!strcasecmp(str, "LIST")) return LIST;
    if(!strcasecmp(str, "INIT")) return INIT;
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
    default:
        return "UNKNOWN";
    }
}

void print_msg(msg m){
    printf("\n---Message---\n");
    printf("Client id: %d\n", m.client_id);
    printf("Message type: %s\n", parse_type_to_str(m.type));
    printf("Content: %s\n", m.content);
    printf("---------------\n");
}

