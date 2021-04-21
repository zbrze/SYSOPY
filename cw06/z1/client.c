#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include "common_header.h"
int client_queue_id = -1;
int server_queue_id = -1;
int client_id;
void client_stop(){
    msg stop_msg;
    stop_msg.type = STOP;
    stop_msg.client_id = client_id;
    strcpy(stop_msg.content, "Hi, I want to go home");

    if(msgsnd(server_queue_id, &stop_msg, max_msg_size,0) == -1){
        perror("Sending STOP message failed\n");
    }
    if(msgctl(client_queue_id,IPC_RMID, NULL) == -1){
        perror("Unable to delete msg queue\n");
    }
    printf("Client closed\n");
}

void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("received sigint\n");
    exit(0);

}
void client_start(){
    srand(time(NULL));
    key_t queue_key;
    if((queue_key= ftok(KEY_PATH, (rand() % 100 + 1)))== -1){
        perror("Ftok failure");
        exit(-1);
    }

    if((client_queue_id = msgget(queue_key, IPC_CREAT | 0666)) == -1){
        perror("Msgget failure");
        exit(-1);
    }

    key_t server_queue_key;
    if((server_queue_key = ftok(KEY_PATH, KEY_GEN)) == -1){
        perror("Failed to get server's queue key");
        exit(-1);
    }

    if((server_queue_id = msgget(server_queue_key, 0)) == -1){
        perror("Failed to get server's queue id");
        exit(-1);
    }
     if(atexit(client_stop) == -1){
         perror("Failed to set exit");
         exit(-1);
     }
    struct sigaction action;
    action.sa_sigaction = sigint_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    printf("Succesfully started client:\n client queue: %d\n Im sending INIT message to server\n", client_queue_id);

    msg init_msg;
    init_msg.type = INIT;
    sprintf(init_msg.content, "%d", client_queue_id);
    if(msgsnd(server_queue_id, &init_msg, max_msg_size, 0) == -1){
        perror("Failed to send init msg");
        exit(0);
    }
    sleep(0.5);
    msg server_reply;
    if(msgrcv(client_queue_id, &server_reply, max_msg_size, INIT, 0) == -1){
        perror("Failed to receive reply from server");
        exit(-1);
    }

    print_msg(server_reply);

    client_id = atoi(server_reply.content);

    if(client_id == -1){
        printf("Seems that there is no place for me.\nGonna stop working\n");
        exit(0);
    }
    printf("Succesfully connected to server\nMy new id: %d\n", client_id);
}

void send_list_rqst(){
    msg list_msg;
    list_msg.type = LIST;
    list_msg.client_id = client_id;

    if(msgsnd(server_queue_id, &list_msg, max_msg_size, 0) == -1){
        perror("Sending list request failed");
        exit(-1);
    }

    msg list_reply;
    if(msgrcv(client_queue_id, &list_reply, max_msg_size, LIST, 0) == -1){
        perror("Receiving msg from server failure");
        exit(-1);
    }
    printf("Received list:\n%s\n", list_reply.content);
    printf("\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
}
int main(int argc, char**argv){
    client_start();
    printf("Avaliable commands are STOP, CONNECT other_client_id, LIST\n"); //disconnect and send msg avaliable in chat mode
    char *command_line = NULL; 
    size_t command_len = 0;
    while(1){
        if(getline(&command_line, &command_len, stdin) != -1){
            command_line[strlen(command_line)-1] = '\0';
            printf("%s", command_line);
            char* command = strtok_r(command_line, " ", &command_line);
            char* id = strtok_r(command_line, " ", &command_line);

            printf("\n%s  parsed:%d\n", command, parse_str_to_type(command));
            switch (parse_str_to_type(command))
            {
            case STOP:
                exit(0);
            
            case LIST:
                printf("Sending list request\n");
                send_list_rqst();
                break;
            
            case CONNECT:
                printf("\nid: %s\n", id);
                break;
        
            
            default:
                printf("Unknown command\n");
                break;
            }
        }
        sleep(0.2);
    }
}