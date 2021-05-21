#include "header.h"

int str_to_mode(char* str){
    if(!strcasecmp(str, "numbers")) return NUMBERS;
    if(!strcasecmp(str, "blocks")) return BLOCK;
    return -1;
}

char* mode_to_str(int mode){
    char* str = calloc(12, sizeof(char));
    switch (mode)
    {
    case NUMBERS:
        strcpy(str, "sign");
        break;
    case BLOCK:
        strcpy(str, "blocks");
        break;
    default:
        strcpy(str, "wrong");
        break;
    }
    return str;
}

void exit_error(char *content){
    perror(content);
    exit(-1);
}
