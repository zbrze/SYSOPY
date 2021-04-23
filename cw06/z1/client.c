#include "common_header.h"
int client_queue_id = -1;
int server_queue_id = -1;
int client_id = -1;
int inputAvailable();  
void get_received_msg();
void send_disconnect();
int chatting = 0;
void client_stop(){
    if(chatting){
        //sending disconnect to my chatting mate
        send_disconnect();
    }
    if(client_id != -1){
        printf("Sending STOP to server\n");
        msg stop_msg;
        stop_msg.type = STOP;
        stop_msg.client_id = client_id;
        stop_msg.failed = 0;
        strcpy(stop_msg.content, "Hi, I want to go home");

        if(msgsnd(server_queue_id, &stop_msg, max_msg_size,0) == -1){
            exit_error("Sending STOP message failed\n"); 
        }
    }
    
    if(msgctl(client_queue_id,IPC_RMID, NULL) == -1){
        perror("Unable to delete msg queue\n");
    }
    printf("Client closed\n");
}

int msgrcv_with_timeout(int client_queue_id, msg* server_reply, int timeout){
    int waited = 0;
    int ret_val;
    while(waited < timeout){
        ret_val = msgrcv(client_queue_id, server_reply, max_msg_size, INIT, IPC_NOWAIT);
        if(ret_val != -1){
            break;
        }
        waited += 500;
        usleep(500);
    }
    return ret_val;
}

void client_start(){
    srand(time(NULL));
    key_t queue_key;
    if((queue_key= ftok(KEY_PATH, (rand() % 100 + 1)))== -1){
        exit_error("Ftok failure");
    }

    if((client_queue_id = msgget(queue_key, IPC_CREAT | 0666)) == -1){
        exit_error("Msgget failure");
    }

    key_t server_queue_key;
    if((server_queue_key = ftok(KEY_PATH, KEY_GEN)) == -1){
        exit_error("Failed to get server's queue key");
    }

    if((server_queue_id = msgget(server_queue_key, 0)) == -1){
        exit_error("Failed to get server's queue id");
    }
     if(atexit(client_stop) == -1){
         exit_error("Failed to set exit");
     }

    set_sigint_handling(sigint_handler);

    printf("Succesfully started client:\n client queue: %d\n Im sending INIT message to server\n", client_queue_id);

    msg init_msg;
    init_msg.type = INIT;
    init_msg.failed = 0;
    sprintf(init_msg.content, "%d", client_queue_id);
    print_msg(init_msg);
    if((msgsnd(server_queue_id, &init_msg, max_msg_size, 0)) == -1){
        exit_error("Failed to send init msg");
    }

    msg server_reply;

    if(msgrcv_with_timeout(client_queue_id, &server_reply, max_wait_for_init) == -1){
        exit_error("Failed to receive reply from server");
    }
    
    print_msg(server_reply);

    client_id = atoi(server_reply.content);

    if(server_reply.failed == 1){
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
        exit_error("Sending list request failed");
    }

    msg list_reply;
    if(msgrcv(client_queue_id, &list_reply, max_msg_size, LIST, 0) == -1){
        exit_error("Receiving msg from server failure");
    }
    printf("Received list:\n%s\n", list_reply.content);
    printf("\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
}

void send_disconnect(){
    msg disconnect_msg;
    disconnect_msg.type = DISCONNECT;
    disconnect_msg.client_id = client_id;
    disconnect_msg.failed = 0;
    strcpy(disconnect_msg.content, "Hi, I wanna stop chatting");
    if(msgsnd(server_queue_id, &disconnect_msg, max_msg_size, 0) == -1){
        exit_error("Sending disconnect msg failure");
    }
    chatting = 0;
    printf("You left the chat\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
}

void chat_room(int interlocutor_queue, int interlocutor_id){
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
                printf("command: %s, parsed to: %d", command, parse_str_to_type(command));
                if(parse_str_to_type(command) == DISCONNECT){
                    send_disconnect();
                    break;
                }

                if(parse_str_to_type(command) == MSG){
                    printf("\nsending: %s  content %s\n", command, content);
                    msg dm;
                    dm.type = MSG;
                    dm.failed = 0;
                    dm.client_id = client_id;
                    strcpy(dm.content, content);
                    if(msgsnd(interlocutor_queue, &dm, max_msg_size, 0) == -1){
                        exit_error("Filed to send dm in chat");
                    }
                    fflush(stdout);

                }

            
          }
        fflush(stdout);
        }
    }
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
    msg connect_rqst;
    connect_rqst.type = CONNECT;
    connect_rqst.client_id = client_id;
    sprintf(connect_rqst.content, "%d", interlocutor_id);
    if(msgsnd(server_queue_id, &connect_rqst, max_msg_size, 0) == -1){
        exit_error("Sending connect request failure");
    }

    msg reply;
    if(msgrcv(client_queue_id, &reply, max_msg_size, CONNECT, 0) == -1){
        perror("Failed to receive reply from server");
        return;
    }
    if(reply.failed){
        printf("My friend cant chat with me :<\n");
        return;
    }
    chatting = 1;
    chat_room(atoi(reply.content),interlocutor_id);

}

void get_received_msg(){
    msg received_msg;
    if(msgrcv(client_queue_id, &received_msg, max_msg_size, 0 , IPC_NOWAIT) == -1){ 
        return;
    } 
    if(received_msg.type == STOP){
        print_msg(received_msg);
        printf("\n\nSeems that server ended working\nImma head out\n");
        exit(0);
    }
    if(received_msg.type == CONNECT){
        print_msg(received_msg);
        printf("\nSeems that %d wants to chat with me.\nLets go to the chat room\n\n", received_msg.client_id);
        chatting = 1;
        chat_room(atoi(received_msg.content), received_msg.client_id);
        return;
    }
    if(received_msg.type == DISCONNECT){
        printf("Your interlocutor left the chat\nAvaliable commands are STOP, CONNECT other_client_id, LIST\n");
        fflush(stdout);
        chatting = 0;
    }
    if(received_msg.type == MSG){
        print_msg(received_msg);
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
    }
return 0;
}
