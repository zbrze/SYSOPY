#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv){
    if(argc != 5){
        printf("Wrong number of arguments");
        return -1;
    }
    char* path_pipe = argv[1];
    char* row_no = argv[2];
    char* path_file = argv[3];
    int N = atoi(argv[4]);

    printf("Producers pid: %d\n", getpid());

    FILE *fd_file = fopen(path_pipe, "w");
    FILE *file = fopen(path_file, "r");

    if(file == NULL || fd_file == NULL){
        printf("File opening failure");
        return -1;  
    }
     
    srand(time(NULL));

    char buff[N+1];
    while(fgets(buff, N + 1, file) != NULL){ 
        sleep((rand() % 2) + 1);
        printf("PRODUCER: %s #%s#\n", buff, row_no);
        fprintf(fd_file, "%s #%s#\n", buff, row_no);
    }
    fclose(file);
    if(fclose(fd_file) == -1){
        printf("Pipe closing failure");
        return -1;
    }
    return 0;
}
