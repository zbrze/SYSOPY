#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char** argv){
    if(argc != 4){
        printf("Wrong number of arguments");
        return -1;
    }
    char* path_pipe = argv[1];
    char* path_file = argv[2];
    int N = atoi(argv[3]);

    FILE *fd_file = fopen(path_pipe, "r");
    FILE *file = fopen(path_file, "w");

    if(file == NULL || fd_file == NULL){
        printf("File opening failure");
        return -1;  
    }

    char buffer[N+10];

    while(fgets(buffer, N+10, fd_file) != NULL){
        printf("%s\n", buffer);
        fprintf(file, buffer, strlen(buffer));
    }

    fclose(fd_file);
    fclose(file);
}