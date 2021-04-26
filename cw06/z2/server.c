
#include "common_header.h"
mqd_t server_queue_id;
client clients[max_clients];


void server_stop(){
char reply[max_msg_len];
unsigned int type;
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            if(mq_send(clients[i].queue_id, "Hi, server there. Im closing", max_msg_len, STOP) == -1){
                exit_error("Sending stop server msg failure");
            }
            if(mq_receive(server_queue_id, reply, max_msg_len, &type) == -1){
                perror("Receving reply from client failure");
            }
            printf("\nClient %d disconnected from server\n", clients[i].client_id);
        }
    }

    if(mq_close(server_queue_id) == -1){
        exit_error("Unable to close msg queue");
    }

    if(mq_unlink(SERVER_QUEUE) == -1){
        exit_error("Unable to delete msq queue");
    }

    printf("\nServer closed\n");
}



void server_start(){
    

    server_queue_id =  create_queue(10, 0, SERVER_QUEUE);

    if(atexit(server_stop) != 0) exit_error("Atexit setting failure");

    set_sigint_handling(sigint_handler);

    for(int i = 0; i < max_clients; i++){
        clients[i].client_id = -1;
        clients[i].queue_id = -1;
        clients[i].interlocutor = -1;
        strcpy(clients[i].queue_name, "");
    }
    printf("Server initialized\nMy queue id: %d\n", server_queue_id);
}

void received_stop(int client_id){
    printf("Client %d announced stoping working\n", client_id);
    if(clients[client_id].client_id == -1){
        exit_error("Client not found");
        return;
    }
    if(mq_close(clients[client_id].queue_id) == -1){
        exit_error("Client's queue closing failure");
    }
    clients[client_id].client_id = -1;
    clients[client_id].queue_id = -1;
    printf("Client %d succesfully stopped\n\n", client_id);
}

void received_disconnect(int sender_id){
    int idx_to_discnct = clients[sender_id].client_id;
    printf("Client %d announced disconnecting from chat with %d\n", idx_to_discnct, clients[idx_to_discnct].interlocutor);
    
    if(idx_to_discnct == -1){
        perror("Client doesn't exist");
        return;
    }
    if(clients[idx_to_discnct].interlocutor == -1){
        perror("Client already disconected");
        return;
    }

    int idx_interlocutor = clients[idx_to_discnct].interlocutor; //looking for clients interlocutor to disconnect him too
    if(idx_interlocutor == -1 ||clients[idx_interlocutor].interlocutor == -1){
        perror("Interlocutor doesnt exist or is not connected back");
        return;
    }
    int queue_interlocutor = clients[idx_interlocutor].queue_id;
    char msg_to_interlocutor[max_msg_len];
    if(mq_send(queue_interlocutor, msg_to_interlocutor, max_msg_len, DISCONNECT) == -1){
        exit_error("Msgsnd failure: cannot send message to interlocutor");
    }
    clients[idx_to_discnct].interlocutor = -2;
    clients[idx_interlocutor].interlocutor = -2;
    printf("Clients %d and %d disconnected succesfully\n", sender_id, idx_interlocutor);
}

void received_list(int sender_id){
    int idx_snd_list = sender_id;
    if(idx_snd_list == -1){
        exit_error("Client not found");
    }
    int queue_list = clients[idx_snd_list].queue_id;
    
    char* buff = malloc(sizeof(char) * max_msg_len);
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            sprintf(buff, "%s\nclient: %d, queue: %s, is %savaliable to chat %s\n", buff, clients[i].client_id, clients[i].queue_name, (clients[i].interlocutor < 0) ? "" : "NOT ", (clients[i].client_id == sender_id) ? "- it's you!" : "");
        }
    }
    if(mq_send(queue_list, buff, max_msg_len, LIST) == -1){
        exit_error("Msgsnd failure: cannot send message with list to client");
    }
    free(buff);

    printf("Message with list sent successfully to %d\n", sender_id);
}


void received_connect(char *msg){
    int id_sender, id_interlocutor;
    sscanf(msg, "%d %d", &id_sender, &id_interlocutor);
    int idx_sender = clients[id_sender].client_id;
    int idx_interlocutor = clients[id_interlocutor].client_id;

    printf("Sender %d who is %d wants to chat with %d who is %d\n", idx_sender, clients[idx_sender].interlocutor, idx_interlocutor, clients[idx_interlocutor].interlocutor);
    //start writing msg back to sender
    char* reply_to_sender;
    int failed = 0;
    if(idx_interlocutor == -1 || clients[idx_sender].client_id == -1 || clients[idx_interlocutor].client_id == -1 || idx_interlocutor == idx_sender){
        printf("Sender or client with whom sender wants to connect not found (or you want to connect with yourself - not good");
        failed = 1;
    }else if(clients[idx_interlocutor].interlocutor >= 0){
        printf("Cannot connect: %d is chatting with %d", clients[idx_interlocutor].client_id, clients[idx_interlocutor].interlocutor);
        failed = 1;
    }
    int sender_queue = clients[idx_sender].queue_id;
    int interlocutor_queue = clients[idx_interlocutor].queue_id;
    

    if(failed){
        reply_to_sender = malloc(2*sizeof(char));
        strcpy(reply_to_sender, "-1");
        //sending msg with failure reason in content
        if(mq_send(sender_queue, reply_to_sender, max_msg_len, CONNECT) == -1){
            free(reply_to_sender);
            exit_error("Sending refusal reply to connect rqst failure");
        }
        free(reply_to_sender);
        return;
    }

    //setting sender and interlocutor busy chatting with each other
    clients[idx_interlocutor].interlocutor = idx_sender;
    clients[idx_sender].interlocutor = idx_interlocutor;
    reply_to_sender = malloc(sizeof(char)* max_msg_len);
    sprintf(reply_to_sender, "%s",clients[idx_interlocutor].queue_name);  
    
    if(mq_send(sender_queue, reply_to_sender, max_msg_len, CONNECT) == -1){
        free(reply_to_sender);
        exit_error("Msgsnd failed: cannot send connect reply to sender");
    }
    free(reply_to_sender);
    char* msg_to_interlocutor = malloc(sizeof(char)* max_msg_len);
    sprintf(msg_to_interlocutor, "%d %s",clients[idx_sender].client_id, clients[idx_sender].queue_name);

    if(mq_send(interlocutor_queue, msg_to_interlocutor, max_msg_len, CONNECT) == -1){
        free(msg_to_interlocutor);
        exit_error("Msgsnd failed: cannot send msg to interlocutor");
    }
    free(msg_to_interlocutor);
    printf("Succesfully connected %d and %d\n", idx_sender, idx_interlocutor);
}

int get_free_index(){
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id == -1) return i;
    }
    return -1;
}

void received_init(char* msg){
    printf("Adding client\n");
    int idx_new = get_free_index();
    if(idx_new == -1){
        perror("No free places!");
        return;
    }
    mqd_t new_queue_id;
    char queue_name[10];
    sscanf(msg, "%s", queue_name);

    if((new_queue_id = mq_open(queue_name, O_RDWR)) == -1){
        printf("%d", errno);
        exit_error("Queue opening failure");
    }
    char reply[2];
    sprintf(reply, "%d", idx_new);

    if(mq_send(new_queue_id, reply, max_msg_len, 0) == -1){
        perror("Msgsnd failed: new client sent me wrong queue id\n");
        return; // dont wanna stop all server because of one weirdo
    }
    else{
        clients[idx_new].client_id = idx_new;
        clients[idx_new].queue_id = new_queue_id;
        clients[idx_new].interlocutor = -1;
        strcpy(clients[idx_new].queue_name, queue_name);
    }
    printf("Succesfully added client %d of queue: %d (name : %s)\n", clients[idx_new].client_id, clients[idx_new].queue_id, queue_name);
    

}

int main(){

    server_start();
    char received[max_msg_len];
    int client_id;
    while(1){
        unsigned int type;
        if(mq_receive(server_queue_id, received, max_msg_len, &type) < 0){
            perror("Msgrcv failed");
            continue;
        }else{
            sscanf(received, "%d", &client_id);
            switch (type){
            case STOP:
                received_stop(client_id);
                break;
            case DISCONNECT:
                received_disconnect(client_id);
                break;
            case LIST:
                received_list(client_id);
                break;
            case CONNECT:
                received_connect(received);
                break;
            case INIT:
                received_init(received); //here client id is just queue id
                break;
            default:
                printf("Unknow type of received message");
                break;
            }
        
        fflush(stdout);
        }
        usleep(1000); // prevent processor hoarding
    }
    
    return 0;
}