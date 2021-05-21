#include "header.h"

int rand_sleep(int min, int max){
    min *= 1000000;
    max *= 1000000;
    return ((rand() % (max - min) + min));
}

void exit_error(char *content){
    perror(content);
    exit(-1);
}