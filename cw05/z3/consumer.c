#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/file.h>
int N = 0;
int max_line_nr = 5;
void insert_in_line(FILE* file, int line, char* producer_line) {
    
    rewind(file);

    int nline = 1;
    char c;
    int buff_size = N + 10;
    size_t len = 0;
    size_t longest_line = 0, curr_len = 0;
    while(fread(&c, sizeof(char), 1, file) ==1){
        len ++;
        curr_len++;
        if(len == buff_size) buff_size *=2;
        if(c == '\n'){ 
            nline++;
        if(curr_len > longest_line){ 
                longest_line = curr_len;
            }
        curr_len = 0;
        }
    }
    rewind(file);
    
    if(nline >= line){
        int l = 1;
        int position;
        char* buff_after_line = malloc(sizeof(char)*len);
        char* buff_line = malloc(sizeof(char) * longest_line);
        while(getline(&buff_line, &longest_line, file) > 0){
            if(l == line){
                fseek(file, -1, SEEK_CUR);
                position = ftell(file);
                fseek(file, 1, SEEK_CUR);

            }else if (l > line){
                sprintf(buff_after_line, "%s%s", buff_after_line, buff_line);
            }
            l++;
        }
        fseek(file, position, SEEK_SET);
        fprintf(file, " %s\n%s", producer_line, buff_after_line);
    }else{
        fseek(file, -1, SEEK_END);
        for(int i = 0; i < line - nline; i++){
            fprintf(file, "\n");
        }
        fprintf(file, "%s", producer_line);
        fseek(file, 1, SEEK_CUR);
    }
}


int find_line(char* buffer){
    int i = 0;
    while(i < strlen(buffer)){
        if(buffer[i] == '#'){
            i++;
            break;
        }
        i++;
    }
    char* line_nr = malloc(sizeof(char) * (strlen(buffer) - i));

    while(isdigit(buffer[i])){
        sprintf(line_nr, "%s%c", line_nr, buffer[i]);
        i++;
    }
    return atoi(line_nr);
}
int main(int argc, char** argv){
    if(argc != 4){
        printf("Wrong number of arguments");
        return -1;
    }
    char* path_pipe = argv[1];
    char* path_file = argv[2];
    N = atoi(argv[3]);

    FILE *fd_file = fopen(path_pipe, "r");
    FILE *file = fopen(path_file, "r+");
    
    if(file == NULL || fd_file == NULL){
        printf("File opening failure");
        return -1;  
    }

    char buffer[N+6];

    while(fgets(buffer, N+6, fd_file) != NULL){
        printf("Consumer: %s\n", buffer);
        int line_no = find_line(buffer);
        int lock;
       // if ((lock = flock(fileno(file), LOCK_EX)) < 0) return -1;
        insert_in_line(file, line_no, buffer);
        //if ((lock = flock(fileno(file), LOCK_UN)) < 0) return -1;
    }

    fclose(fd_file);
    fclose(file);
}