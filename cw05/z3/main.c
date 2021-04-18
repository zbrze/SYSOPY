
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>


int main(int argc, char** argv){

    if(mkfifo("pipe", 0666) < 0){
        printf("Cannot create named pipe.\n");
        exit(EXIT_FAILURE);
    }
    if(argc != 2){
        printf("Wrong number of arguments");
        return -1;
    }
    int producers_no = atoi(argv[1]);
    for(unsigned int i = 0; i < producers_no; i++){
        char file_name[20];
        sprintf(file_name, "producer%dfile.txt", i);
        FILE* f = fopen(file_name, "w");
        char *line = malloc(33* sizeof(char));
        for(int j = 0; j < 30; j++){ 
            line[j] = 120 - i;
        }
        fprintf(f, "%s", line);
        printf("%s %d\n", line, getpid());
        fclose(f);
        free(line);      

    }

    for(int i = 0 ; i < producers_no; i ++){
        if(fork() == 0){
            char buff[10];
            sprintf(buff, "%d", i + 1);
            char file_name[20];
            sprintf(file_name, "producer%dfile.txt", i);
            execl("./producer", "./producer", "pipe",buff , file_name, "10", NULL);
            printf("Producer failure");
            exit(3);
        }
        sleep(1);
    }
    if(fork() == 0){
        execlp("./consumer", "./consumer", "pipe", "consumer.txt" , "10", NULL);
        exit(3);
    }
    wait(NULL);
    for(int i = 0; i < producers_no; i++) wait(NULL);


    return 0;
}