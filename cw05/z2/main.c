#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <unistd.h>
int send_mode = 0;
int max_mail_len = 2000;
int max_output_line = 500;
int main(int argc, char** argv){
    int sender = 0;
    int data = 0;
    char* mail_address = NULL;
    char* title = NULL;
    char body[max_mail_len];
    for(int i = 0; i < max_mail_len; i++) body[i] = '\0';
    if(argc == 2){
        if(!strcmp(argv[1],"nadawca")) sender = 1;
        else if(!strcmp(argv[1],"data")) data = 1;
        else{
            printf("Wrong arguments");
            return -1;
        }

    }else if(argc >= 4){
        mail_address = argv[1];
        title = argv[2];
        int i = 3;
        while(argc - i > 0 && i < max_mail_len){
            sprintf(body,"%s %s", body, argv[i]);
            i++;
        }
        send_mode = 1;
    }else{
        printf("Wrong numbers of arguments: %d", argc);
        return -1;
    }
    FILE *email_list;
    char *command;
    if(send_mode == 1){
        command = malloc(sizeof(char)* max_mail_len);
        sprintf(command, "echo '%s' | mail -s '%s' %s", title, body, mail_address);
        printf("%s\n", command);
        email_list = popen(command, "r");
        if (email_list == NULL){
            printf("Popen failed");
            exit(-1);
        }
        //nie ma zwrotnej wiadomości
        printf("mail send to receiver\n");
    }else{
        if(sender)command = "echo | mail -f /var/mail/zuzanna | awk '1;/Message/{exit}' | sed '1d;2d;3d;$d' | sort -k2";
        else if(data) command = " echo | mail -f /var/mail/zuzanna | awk '1;/Message/{exit}' | sed '1d;2d;3d;$d' | sort -k4M -k5 -k6";
        printf("sorted by: \n");
        email_list = popen(command, "r");
            if (email_list == NULL){
                printf("Popen failed");
                exit(1);
            }
        char line[max_output_line];
        while(fgets(line, max_output_line, email_list) != NULL){
        printf("%s", line);
        }
    }
        
      

    return 0;
}
