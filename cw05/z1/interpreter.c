#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int max_commands = 10;
int max_command_len = 300;
char** divide_command(char* a_command){

    char** commands = malloc(max_command_len* sizeof(char*));
    for(int i = 0; i < max_command_len; i++) commands[i]= NULL;
    char* command;
    int index = 0;
    command = strtok_r(a_command, " \n\0", &a_command);
    while (command != NULL) {
        commands[index] = malloc(strlen(command) * sizeof(char));
        if(command != NULL && strcmp(command," \n\0") && strcmp(command,"\0") && strcmp(command,"\n")){ 
          strcpy(commands[index], command);
          index++;
          }
          command = strtok_r(a_command, " \n\0", &a_command);
        
    }
    commands[index] = NULL;
    return commands;
}

char*** count_commands(char* line, size_t* len){
    if(len == 0) return NULL;
    int index = 0; 
    char*** commands = malloc(sizeof(char **) * max_commands);
    char* command;
    command = strsep(&line, "|");
    while (command != NULL) {
      char* tmp = malloc(sizeof(char) * max_commands);
        strcpy(tmp,command);
        command = strsep(&line, "|");
        commands[index] = divide_command(tmp);
        index++;
  }
  commands[index] = NULL;

  *len = index;
  return commands;   
    
}


void free_commands(char*** commands, size_t* len){
    for (int i=0; i < *len; i++){
        for (int j=0; j < max_command_len; j++) {
            if(commands[i][j] != NULL)free(commands[i][j]);    
        }
    }
    *len = 0;
}
int main(int argc, char** argv){
    if(argc != 2){
        printf("Wrong number of arguments");
        return -1;
    }
    char* file_name = argv[1];
    FILE* command_file = fopen(file_name, "r");
    if(command_file == NULL){
        printf("Cannot open file with commands");
        return -1;
    }
    char*** commands;
    char *line = NULL;
    size_t len = 0;
    size_t line_len = 0;
    int fd[2], prev_fd[2];
    while((line_len = getline(&line, &len, command_file)) != -1){
        printf("\ncommand %s\n", line);
        commands = count_commands(line, &line_len);
       
        for(int i = 0; i < line_len; i++){
           if (i != line_len - 1)    
                if (pipe(fd) == -1){
                    perror("Pipe failure");
                    exit(3);
                }

          pid_t pid = fork();

            if (pid < 0){
                perror("Cannot fork.");
            }

            else if (pid == 0){
              if (i != 0){
                    dup2(prev_fd[0], STDIN_FILENO);
                    close(prev_fd[1]); 
                }

                dup2(fd[1], STDOUT_FILENO);     
                close(fd[0]);        
                
              execvp(commands[i][0], commands[i]);
              fprintf(stderr,"execvp(%s) failed\n", commands[i][0]);
              exit(0);
            } else{
                if (i != 0){              
                    close(prev_fd[1]);      
                }
                prev_fd[0] = fd[0];
                prev_fd[1] = fd[1];
            }

        }
        for (int i=0; i <= line_len; i++)  wait(NULL);
        free_commands(commands, &line_len);
    }
    free(commands);
    fclose(command_file);
    return 0;
    
}
