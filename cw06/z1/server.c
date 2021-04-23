#include "common_header.h"
#define init_msg(X) msg X = {.failed = 0, .client_id = -1}
int server_queue_id;
client clients[max_clients];
void server_stop(){

    init_msg(closing_server_msg);
    closing_server_msg.type = STOP;
    strcpy(closing_server_msg.content, "Hi, server here. Im closing!");
    print_msg(closing_server_msg);

    msg msgs_from_clients;

    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            if(msgsnd(clients[i].queue_id, &closing_server_msg, max_msg_size, 0) == -1){
                exit_error("Sending stop server msg failure");
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



void server_start(){
    key_t queue_key;
    if((queue_key= ftok(KEY_PATH, KEY_GEN))== -1){
        exit_error("Ftok failure");
    }

    if((server_queue_id = msgget(queue_key, IPC_CREAT  | 0666)) == -1){
        exit_error("Msgget failure");
    }

    if(atexit(server_stop));

    set_sigint_handling(sigint_handler);

    for(int i = 0; i < max_clients; i++){
        clients[i].client_id = -1;
        clients[i].queue_id = -1;
        clients[i].interlocutor = -1;
    }
    printf("Server initialized\nMy queue id: %d\n", server_queue_id);
}


void received_stop(msg *message){
    printf("Client %d announced stoping working\n", message->client_id);
    print_msg(*message);
    int idx_to_del = message->client_id;
    if(idx_to_del == -1){
        perror("Client not found");
        return;
    }
    clients[idx_to_del].client_id = -1;
    clients[idx_to_del].queue_id = -1;
    printf("Client %d succesfully stopped\n\n", message->client_id);
}

void received_disconnect(int sender_id){
    printf("Client %d announced disconnecting\n", sender_id);
    int idx_to_discnct = sender_id;
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
    msg msg_to_interlocutor;
    msg_to_interlocutor.type = DISCONNECT;
    strcpy(msg_to_interlocutor.content, "Hi, I gotta go, bye!");
    msg_to_interlocutor.client_id = sender_id;

    if(msgsnd(queue_interlocutor, &msg_to_interlocutor, max_msg_size, 0) == -1){
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
    
    init_msg(list_msg);
    list_msg.type = LIST;

    char* buff = malloc(sizeof(char) * max_msg_len);
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id != -1){
            sprintf(buff, "%s\nclient: %d, is %savaliable to chat %s\n", buff, clients[i].client_id, (clients[i].interlocutor < 0) ? "" : "NOT ", (clients[i].client_id == sender_id) ? "- it's you!" : "");
        }
    }
    strcpy(list_msg.content, buff);
    if(msgsnd(queue_list, &list_msg, max_msg_size, 0) == -1){
        exit_error("Msgsnd failure: cannot send message with list to client");
    }
    printf("Message with list sent successfully to %d\n", sender_id);
}

void received_connect(msg *message){
    print_msg(*message);
    int idx_sender = message->client_id;
    int idx_interlocutor = atoi(message->content);

    printf("Sender %d who is %d wants to chat with %d who is %d\n", idx_sender, clients[idx_sender].interlocutor, idx_interlocutor, clients[idx_interlocutor].interlocutor);
    //start writing msg back to sender
    init_msg(msg_to_sender);
    msg_to_sender.type = CONNECT;

    if(idx_interlocutor == -1 || clients[idx_sender].client_id == -1 || clients[idx_interlocutor].client_id == -1 || idx_interlocutor == idx_sender){
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
            exit_error("Sending refusal reply to connect rqst failure");
        }
        return;
    }

    //setting sender and interlocutor busy chatting with each other
    clients[idx_interlocutor].interlocutor = idx_sender;
    clients[idx_sender].interlocutor = idx_interlocutor;

    sprintf(msg_to_sender.content, "%d", interlocutor_queue);
    msg_to_sender.client_id = idx_interlocutor;     
    
    if(msgsnd(sender_queue, &msg_to_sender, max_msg_size, 0) == -1){
        exit_error("Msgsnd failed: cannot send connect reply to sender");
    }

    init_msg(msg_to_interlocutor);
    msg_to_interlocutor.type = CONNECT;
    msg_to_interlocutor.client_id = idx_sender;
    sprintf(msg_to_interlocutor.content, "%d", sender_queue);

    if(msgsnd(interlocutor_queue, &msg_to_interlocutor, max_msg_size, 0) == -1){
        exit_error("Msgsnd failed: cannot send msg to interlocutor");
    }
    printf("Succesfully connected %d and %d\n", idx_sender, idx_interlocutor);
}

int get_free_index(){
    for(int i = 0; i < max_clients; i++){
        if(clients[i].client_id == -1) return i;
    }
    return -1;
}

void received_init(msg *message){
    msg reply;
    reply.type = INIT;
    printf("Adding client\n");
    int idx_new = get_free_index();
    if(idx_new == -1){
        perror("No free places!");
        reply.failed = 1;
    }
    int new_queue = atoi(message->content);
    sprintf(reply.content, "%d", idx_new);
    print_msg(reply);
    if(msgsnd(new_queue, &reply, max_msg_size, 0) == -1){
        perror("Msgsnd failed: new client sent me wrong queue id\n");
        return; // dont wanna stop all server because of one weirdo
    }
    else{
        clients[idx_new].client_id = idx_new;
        clients[idx_new].queue_id = new_queue;
        clients[idx_new].interlocutor = -1;
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
            continue;
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