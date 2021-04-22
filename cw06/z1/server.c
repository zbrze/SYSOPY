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

int server_queue_id;
client clients[max_clients];
void server_stop(){

    msg closing_server_msg;
    closing_server_msg.type = STOP;
    closing_server_msg.client_id = -1;
    strcpy(closing_server_msg.content, "Hi, server here. Im closing!");
    print_msg(closing_server_msg);

    msg msgs_from_clients;

    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            if(msgsnd(clients[i].queue_id, &closing_server_msg, max_msg_size, 0) == -1){
                perror("Sending stop server msg failure");
            }
            if(msgrcv(server_queue_id, &msgs_from_clients, max_msg_size, STOP, 0) == -1){
                perror("Receving reply from client failure");
            }
            printf("\nClient %d disconnected from server\n", clients[i].client_id);
        }
    }
    

    if(msgctl(server_queue_id,IPC_RMID, NULL) == -1){
        perror("Unable to delete msg queue");
    }

    printf("\nServer closed\n");
}

void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("received sigint");

}

void server_start(){
    key_t queue_key;
    if((queue_key= ftok(KEY_PATH, KEY_GEN))== -1){
        perror("Ftok failure");
        exit(-1);
    }

    if((server_queue_id = msgget(queue_key, IPC_CREAT  | 0666)) == -1){
        perror("Msgget failure");
        exit(-1);
    }

    if(atexit(server_stop));

    struct sigaction action;
    action.sa_sigaction = sigint_handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    for(int i = 0; i < max_clients; i++){
        clients[i].client_id = -1;
        clients[i].queue_id = -1;
        clients[i].interlocutor = -1;
    }
    printf("Server initialized\nMy queue id: %d\n", server_queue_id);
}

int get_client(int wanted_id){
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id == wanted_id) return i; //returning index of wanted client
    }
    return -1;
}

void received_stop(msg *message){
    printf("Client %d announced stoping working\n", message->client_id);
    print_msg(*message);
    int idx_to_del = get_client( message->client_id);
    if(idx_to_del == -1){
        perror("Client not found");
        exit(-1);
    }
    clients[idx_to_del].client_id = -1;
    clients[idx_to_del].queue_id = -1;
    printf("Client %d succesfully stopped\n\n", message->client_id);
}

void received_disconnect(int sender_id){
    printf("Client %d announced disconnecting\n", sender_id);
    int idx_to_discnct = get_client(sender_id);
    if(idx_to_discnct == -1){
        perror("Client doesn't exist");
        exit(-1);
    }
    if(clients[idx_to_discnct].interlocutor == -1){
        perror("Client already disconected");
        exit(-1);
    }

    int idx_interlocutor = get_client(clients[idx_to_discnct].interlocutor); //looking for clients interlocutor to disconnect him too
    if(idx_interlocutor == -1 ||clients[idx_interlocutor].interlocutor == -1){
        perror("Interlocutor doesnt exist or is not connected back");
        exit(-1);
    }
    int queue_interlocutor = clients[idx_interlocutor].queue_id;
    msg msg_to_interlocutor;
    msg_to_interlocutor.type = DISCONNECT;
    strcpy(msg_to_interlocutor.content, "Hi, I gotta go, bye!");
    msg_to_interlocutor.client_id = sender_id;

    if(msgsnd(queue_interlocutor, &msg_to_interlocutor, max_msg_size, 0) == -1){
        perror("Msgsnd failure: cannot send message to interlocutor");
        exit(-1);
    }
    clients[idx_to_discnct].interlocutor = -2;
    clients[idx_interlocutor].interlocutor = -2;
    printf("Clients %d and %d disconnected succesfully\n", sender_id, idx_interlocutor);
}

void received_list(int sender_id){
    int idx_snd_list = get_client(sender_id);
    if(idx_snd_list == -1){
        perror("Client not found");
        exit(-1);
    }
    int queue_list = clients[idx_snd_list].queue_id;
    
    msg list_msg;
    list_msg.type = LIST;
    list_msg.failed = 0;

    char* buff = malloc(sizeof(char) * max_msg_len);
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            sprintf(buff, "%s\nclient: %d, is %savaliable to chat\n", buff, clients[i].client_id, (clients[i].interlocutor < 0) ? "" : "NOT ");
        }
    }
    strcpy(list_msg.content, buff);
    if(msgsnd(queue_list, &list_msg, max_msg_size, 0) == -1){
        perror("Msgsnd failure: cannot send message with list to client");
        exit(-1);
    }
    printf("Message with list sent successfully to %d\n", sender_id);
}

void received_connect(msg *message){
    print_msg(*message);
    int id_sender = message->client_id;
    int idx_sender = get_client(id_sender);
    int id_interlocutor = atoi(message->content);
    int idx_interlocutor = get_client(id_interlocutor);

    printf("Sender %d who is %d wants to chat with %d who is %d\n", id_sender, clients[idx_sender].interlocutor, id_interlocutor, idx_interlocutor);
    //start writing msg back to sender
    msg msg_to_sender;
    msg_to_sender.type = CONNECT;
    msg_to_sender.failed = 0;

    if( idx_sender == -1 || idx_interlocutor == -1 || idx_interlocutor == idx_sender){
        printf("Sender or client with whom sender wants to connect not found (or you want to connect with yourself - not good");
        sprintf(msg_to_sender.content ,"Sender or client with whom sender wants to connect not found (or you want to connect with yourself - not good");
        msg_to_sender.failed = 1;
    }else if(clients[idx_interlocutor].interlocutor >= 0){
        sprintf(msg_to_sender.content,"Cannot connect: %d is chatting with %d", clients[idx_interlocutor].client_id, clients[idx_interlocutor].interlocutor);
        msg_to_sender.failed = 1;
    }
    int sender_queue = clients[idx_sender].queue_id;
    int interlocutor_queue = clients[idx_interlocutor].queue_id;
    

    if(msg_to_sender.failed){
        msg_to_sender.client_id = -1;
        //sending msg with failure reason in content
        if(msgsnd(sender_queue, &msg_to_sender, max_msg_size, 0) == -1){
            perror("Msgsnd failed");
            exit(-1);
        }
        return;
    }

    //setting sender and interlocutor busy chatting with each other
    clients[idx_interlocutor].interlocutor = id_sender;
    clients[idx_sender].interlocutor = id_interlocutor;

    sprintf(msg_to_sender.content, "%d", interlocutor_queue);
    msg_to_sender.client_id = id_interlocutor;
    
    if(msgsnd(sender_queue, &msg_to_sender, max_msg_size, 0) == -1){
        perror("Msgsnd failed: cannot send msg to sender");
        exit(-1);
    }

    msg msg_to_interlocutor;
    msg_to_interlocutor.type = CONNECT;
    msg_to_interlocutor.client_id = id_sender;
    msg_to_interlocutor.failed = 0;
    sprintf(msg_to_interlocutor.content, "%d", sender_queue);

    if(msgsnd(interlocutor_queue, &msg_to_interlocutor, max_msg_size, 0) == -1){
        perror("Msgsnd failed: cannot send msg to interlocutor");
        exit(-1);
    }
    printf("Succesfully connected %d and %d\n", id_sender, id_interlocutor);
}

int get_free_index(){
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id == -1) return i;
    }
    return -1;
}

void received_init(msg *message){
    printf("Adding client\n");
    int idx_new = get_free_index();
    if(idx_new == -1){
        perror("No free places!");
    }
    clients[idx_new].client_id = idx_new;
    clients[idx_new].queue_id = atoi(message->content);
    clients[idx_new].interlocutor = -1;

    msg reply;
    reply.type = INIT;
    sprintf(reply.content, "%d", idx_new);

    if(msgsnd(clients[idx_new].queue_id, &reply, max_msg_size, 0) == -1){
        perror("Msgsnd failed");
        exit(-1);
    }
    print_msg(*message);
    printf("Succesfully added client %d of queue: %d\n", idx_new, clients[idx_new].queue_id);
    

}

int main(){

    server_start();
    msg received;
    while(1){
        if(msgrcv(server_queue_id, &received, max_msg_size, -10, 0) == -1){
            perror("Msgrcv failed");
            return -1;
        }
        switch (received.type)
        {
        case STOP:
            received_stop(&received);
            break;
        case DISCONNECT:
            received_disconnect(received.client_id);
            break;
        case LIST:
            received_list(received.client_id);
            break;
        case CONNECT:
            received_connect(&received);
            break;
        case INIT:
            received_init(&received);
            break;
        default:
            printf("Unknow type of received message");
            break;
        }
        fflush(stdout);
    }
    
    return 0;
}