#include "common_header.h"
mqd_t client_queue_id = -1;
mqd_t server_queue_id = -1;
int client_id = -1;
int chatting = 0;
char client_queue_name[queue_name_len];
int inputAvailable();  
void get_received_msg();
void send_disconnect();

void client_stop(){
    if(chatting){
        //sending disconnect to my chatting mate
       send_disconnect();
    }

    if(client_id != -1){
        printf("Sending STOP to server\n");
        char stop_msg[max_id_len];
        sprintf(stop_msg, "%d", client_id);
        if(mq_send(server_queue_id, stop_msg, max_msg_len, STOP) == -1){
            exit_error("Sending STOP message failed\n"); 
        }
    }
    
    if(mq_close(client_queue_id) == -1){
        exit_error("Unable to close client's msg queue\n");
    }

    if(mq_close(server_queue_id) == -1){
        exit_error("Unable to close server's msg queue");
    }

    if(mq_unlink(client_queue_name) == -1){
        exit_error("Unable to delete cliet's queue");
    }
    printf("Client closed\n");
}


void generate_queue_name(){

    srand(time(NULL));
    char name[queue_name_len];
    name[0] = '/';
    for(int i = 1; i < queue_name_len -1; i++){
        char rand_letter = 'a' + (random() % 26);
        name[i] = rand_letter;
    }
    name[queue_name_len - 1] = '\0';
    strcpy(client_queue_name, name);
}

void client_start(){
    generate_queue_name();

    if((server_queue_id = mq_open(SERVER_QUEUE, O_RDWR)) == -1){
        exit_error("Failed to get server's queue id");
    }

    client_queue_id = create_queue(10, 1, client_queue_name);
     if(atexit(client_stop) == -1){
         exit_error("Failed to set exit");
     }
         printf("Succesfully started client:\n client queue: %d %s\n Im sending INIT message to server\n", client_queue_id, client_queue_name);

    sleep(1);
    set_sigint_handling(sigint_handler);


    if(mq_send(server_queue_id, client_queue_name, max_msg_len, INIT) == -1){
        exit_error("Failed to send init msg");
    }

    char server_reply[max_msg_len];
    unsigned type;
    if(mq_receive(client_queue_id, server_reply, max_msg_len, &type) == -1){
        exit_error("Failed to receive reply from server");
    }

    sscanf(server_reply, "%d", &client_id);

    if(client_id == -1){
        printf("Theres no place for me\n");
        exit(0);
    }
    printf("Succesfully connected to server\nMy new id: %d\n", client_id);

}

void send_list_rqst(){
    char list_rqst[max_id_len];
    sprintf(list_rqst, "%d", client_id);

    if(mq_send(server_queue_id, list_rqst, max_msg_len, LIST) == -1){
        exit_error("Sending list request failed");
    }

    char list_reply[max_msg_len];
    unsigned type;
    if(mq_receive(client_queue_id, list_reply, max_msg_len, &type) == -1){
        exit_error("Receiving msg from server failure");
    }
    if(type != LIST){
        exit_error("Wrong type of msg - should be LIST");
    }
    printf("Received list:\n%s\n", list_reply);
    printf("\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
    return;
}

void send_disconnect(){
    if(!chatting) return;
    char disconnect_rqst[max_id_len];
    sprintf(disconnect_rqst,"%d", client_id);
    if(mq_send(server_queue_id, disconnect_rqst, max_msg_len, DISCONNECT) == -1){
        exit_error("Sending disconnect msg failure");
    }
    chatting = 0;
    printf("\nYou left the chat\n");
}

void chat_room(char* interlocutor_queue_name, int interlocutor_id){

    mqd_t interlocutor_queue_id;
    if((interlocutor_queue_id = mq_open(interlocutor_queue_name, O_RDWR)) == -1){
        exit_error("Unable to open interlocutor queue");
    } 
    printf("---------Chatting with %d---------\n", interlocutor_id);
    printf("*To disconnect type DISCONNECT*\n");
    printf("*To send message in chat type MSG msg_content*\n\n");
    fflush(stdin);
    fflush(stdout);
    char *chat_msg = NULL; 
    size_t msg_len = 0;
    while(chatting){
        get_received_msg();
        if(inputAvailable()){
          if(getline(&chat_msg, &msg_len, stdin) != -1){
              chat_msg[strlen(chat_msg)-1] = '\0';
                char* command = strtok_r(chat_msg, " ", &chat_msg);
                char* content = chat_msg;
                if(parse_str_to_type(command) == DISCONNECT){
                    send_disconnect();
                    break;
                }
                if(parse_str_to_type(command) == STOP){
                    exit(0);
                }

                if(parse_str_to_type(command) == MSG){
                    printf("\nsending: %s  content %s\n", command, content);
                    if(mq_send(interlocutor_queue_id, content, max_msg_len, MSG) == -1){
                        exit_error("Filed to send dm in chat");
                    }
                    fflush(stdout);

                }
            }
        }
        usleep(1000); // prevent processor hoarding
    }
    if(mq_close(interlocutor_queue_id) == -1){
        exit_error("Unable to close interlocutors queue");
    }
    printf("\nSuccesfully ended chat\n\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
    fflush(stdout);
}


void send_connect_rqst(char* a_interlocutor_id){

    if(a_interlocutor_id == NULL || a_interlocutor_id[0] == '\t'){
        printf("Usage: CONNECT USER_ID\n");
        return;
    }
    int interlocutor_id = -1;
    char* end;
    interlocutor_id = strtol(a_interlocutor_id, &end, 10);
    if(end == a_interlocutor_id){
        printf("USER_ID needs to be a number\n");
        return;
    }
    interlocutor_id = (int)interlocutor_id;
    printf("\nGonna send connect with %d request\n", interlocutor_id);
    char connect_rqst[max_msg_len + queue_name_len + 2];
    sprintf(connect_rqst, "%d %d", client_id, interlocutor_id);
    if(mq_send(server_queue_id, connect_rqst, max_msg_len, CONNECT) == -1){
        exit_error("Sending connect request failure");
    }

    char reply[max_msg_len];
    unsigned int type;
    if(mq_receive(client_queue_id, reply, max_msg_len, &type) == -1){
        perror("Failed to receive reply from server");
        return;
    }
    printf("\nReply from server: %s\n", reply);
    if(!strcmp(reply, "-1") || type != CONNECT){
        printf("My friend cant chat with me :<\n");
        return;
    }
    chatting = 1;
    char interlocutor_queue_name[queue_name_len];
    sscanf(reply, "%s", interlocutor_queue_name);
    chat_room(interlocutor_queue_name, interlocutor_id);
}

void get_received_msg(){
    char received_msg[max_msg_len];
    unsigned int type;
    struct timespec* timespec = (struct timespec*)malloc(sizeof(struct timespec));
    if (mq_timedreceive(client_queue_id, received_msg, max_msg_len, &type, timespec) < 0){ 
        free(timespec);
        return;
    };
    free(timespec);
    if(type == STOP){
        printf("\n\nSeems that server ended working\nImma head out\n");
        exit(0);
    }
    if(type == CONNECT){
        chatting = 1;
        int interlocutor_id;
        char interlocutor_queue_name[10];
        sscanf(received_msg, "%d %s", &interlocutor_id, interlocutor_queue_name);
        printf("\nSeems that %d (queue name: %s) wants to chat with me.\nLets go to the chat room\n\n", interlocutor_id, interlocutor_queue_name);
        chat_room(interlocutor_queue_name, interlocutor_id);
        return;
    }
    if(type == DISCONNECT){
        printf("Your interlocutor left the chat\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
        fflush(stdout);
        chatting = 0;
    }
    if(type == MSG){
        printf("*new chat msg: %s*\n", received_msg);
        fflush(stdout);
    }
}

int inputAvailable(){
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

int main(int argc, char**argv){
    client_start();
    printf("Avaliable commands are STOP, CONNECT other_client_id, LIST\n"); //disconnect and send msg avaliable in chat mode
    char *command_line = NULL; 
    size_t command_len = 0;
    while(1){
        get_received_msg();
        if(inputAvailable()){
            if(getline(&command_line, &command_len, stdin) != -1){
                command_line[strlen(command_line)-1] = '\0';
                char* command = strtok_r(command_line, " ", &command_line);
                char* id = strtok_r(command_line, " ", &command_line);
                printf("\n%s", command);
                switch (parse_str_to_type(command))
                {
                case STOP:
                    exit(0);
                
                case LIST:
                    printf("Sending list request\n");
                    send_list_rqst();
                    break;
                
                case CONNECT:
                    send_connect_rqst(id);
                    break;
            
                default:
                    printf("Unknown command\n");
                    break;
                }
            }
        }
        usleep(1000); // prevent processor hoarding
    }

return 0;
}
