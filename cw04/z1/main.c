#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include "lib.h"

int main(int argc, char **argv) {
//argumenrs: action, numer of signal, fork/exec
    if(argc != 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    char *operation = argv[1];
    int sig = atoi(argv[2]);
    if(!strcmp(operation, "ignore")) ignore(sig);
    else if(!strcmp(operation, "handler")) handler_testing(sig);
    else if(!strcmp(operation, "mask")) mask(sig);
    else if(!strcmp(operation, "pending")) pending(sig);
    else{
         printf("niewlasciwy argument");
         return -1;
    }
    if(!strcmp("fork", argv[3])){
        pid_t child;
        printf("                    ***FORKING***                    \n");
        if((child = fork()) == 0){
            if(strcmp(operation, "pending")){
                printf("Raising signal in child process: %d\n", getpid());
                raise(SIGUSR1); 
            } 
            if(!strcmp(operation, "ignore")) check_if_ignoring(sig);
            if((!strcmp(operation, "mask"))|| (!strcmp(operation, "pending"))) check_if_waiting(sig);
            exit(0);
            } 
        wait(NULL);
    }else if (!strcmp("exec", argv[3])) execl("./exec.o", "./exec.o", argv[1], argv[2] , NULL);
    printf("\n==========================================================\n\n");


    return 0;
}
