#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include <stddef.h>
#include "lib.h"

int main(int argc, char **argv) {
//argumenrs: action, numer of signal
FILE *output = fopen("raport2.txt", "a");
printf("\n                    ***IN EXEC***                    ");
    if(argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }
    char *operation = argv[1];
    int sig = atoi(argv[2]);
    if(!strcmp(operation, "ignore")){
        printf("\nRaising signal in excec: \n");
        raise(sig);
        check_if_ignoring(sig);
                
     }
    else if(!strcmp(operation, "handler")){
        printf("\nRaising signal in excec: \n");
        raise(sig);
    }
    else if(!strcmp(operation, "mask")) {
        
        printf("Raising signal in excec: \n");
        raise(sig);
        check_if_waiting(sig);
    }
    else if(!strcmp(operation, "pending")){
        printf("\nChecking if signal raised in parent is still waiting:\n");
        check_if_waiting(sig);
    }
    else{
         printf("niewlasciwy argument");
         return -1;
    }

    printf("\n==========================================================\n\n");
    return 0;
}
