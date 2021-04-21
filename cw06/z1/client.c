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
int client_id = -1;
void client_stop(){
    msg stop_msg;
    stop_msg.type = STOP;
    stop_msg.client_id = client_id;

    if(msgsnd(server_queue_id, &stop_msg, max_msg_size,0) == -1){
        perror("Sending STOP message failed\n");
    }

    if(msgctl(client_queue_id,IPC_RMID, NULL) == -1){
        perror("Unable to delete msg queue\n");
    }

    printf("Client closed\n");
    exit(0);
}

void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("received sigint\n");
    client_stop();

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

    //msg init_msg
}
int main(int argc, char**argv){
   /* msg msg_client;
    msg_client.type = INIT;
    sprintf(msg_client.content, "%d", client_queue_id);
    print_msg(msg_client);
    
    if(msgsnd(atoi(argv[1]), &msg_client, max_msg_size, 0) == -1){
        perror("cant send message");
        exit(0);
    }
    sleep(8);
    msg_client.type = LIST;
    sprintf(msg_client.content, "%d", client_queue_id);
    print_msg(msg_client);
    
    if(msgsnd(atoi(argv[1]), &msg_client, max_msg_size, 0) == -1){
        perror("cant send message");
        exit(0);
    }
    while(1){
        sleep(5);
    }*/

    client_start();
    client_stop();

}
