#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int max_command = 10;
int max_args_in_command = 10;
char** divide_command(char* a_command){

    char** commands = malloc(256* sizeof(char*));
    for(int i = 0; i < 256; i++) commands[i]= NULL;
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
    char*** commands = malloc(sizeof(char **) * max_command);
    char* command;
    command = strsep(&line, "|");
    while (command != NULL) {
      char* tmp = malloc(sizeof(char) * max_command);
        strcpy(tmp,command);
        command = strsep(&line, "|");
        commands[index] = divide_command(tmp);
        index++;
  }
  commands[index] = NULL;

  *len = index;
  //printf("%s %s", commands[0][0], commands[0][1]);
  return commands;   
    
}


void free_commands(char*** commands, size_t* len){
    for (int i=0; i < *len; i++){
        for (int j=0; j < 256; j++) {
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
        commands = count_commands(line, &line_len);
        printf("LEN %d\n", line_len);
        //execvp(commands[0][0], commands[0]);
        for(int i = 0; i < line_len; i++){
           if (i < line_len - 1)      // Otwieramy potok przed kazdym programem z wyjatkiem ostatniego
                if (pipe(fd) == -1){
                    fprintf(stderr,"Wywolanie pipe() sie nie powiodlo!\n");
                    exit(3);
                }

          pid_t pid = fork();

            if (pid < 0){
                perror("Cannot fork.");
            }
            else if (pid == 0){
              if (i-1 >= 0){
                    dup2(prev_fd[0], STDIN_FILENO); // Podmieniamy standardowe wejscie na poprzedni potok
                    close(prev_fd[1]);      // Nie bedzie tutaj nic pisac do poprzedniego potoku, tylko czytac
                }


                if (i < line_len - 1){
                    dup2(fd[1], STDOUT_FILENO);     // Podmieniamy standardowe wyjscie na nowy potok
                    close(fd[0]);        // Nie bedzie stad nic czytac z nowego potoku, tylko pisac
                }
              printf("%d %s %s\n", line_len, commands[i][0],  commands[i][1]);
              execvp(commands[i][0], commands[i]);
              printf("\n\n\n");
            } else{
                if (i-1 >= 0){               // Zamykamy tylko te, ktore sa nieuzywane (zamykanie wczesniej spowoduje, ze potomek dostanie zamkniety potok)
                    close(prev_fd[0]);      // Proces macierzysty nie uzywa potokow, wiec zamyka obie koncowki
                    close(prev_fd[1]);      // Ale zamykamy tylko te, ktore sa juz nie uzywane    
                }
                prev_fd[0] = fd[0];
                prev_fd[1] = fd[1];
            }

        }
         for (int i=0; i <= line_len; i++)   // Proces macierzysty czeka na wszystkich potomkow
            wait(NULL);
        
            free_commands(commands, &line_len);
    }
    free(commands);
    
    fclose(command_file);
    return 0;
    
}