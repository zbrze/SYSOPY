#include "header.h"
char* name;
int connection_type;
char* server_addr;
int server_fd;
char symbol;
struct sockaddr_un loc_addr;
struct sockaddr_in net_addr;

void connect_to_server(char* port_no){

    if(connection_type == LOCAL){
        printf("Connectiong to local socket\n");
        loc_addr.sun_family = AF_UNIX;
        strcpy(loc_addr.sun_path, server_addr);

        if((server_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) exit_error("Socket to server failed.");

        struct sockaddr_un client_addr;
        client_addr.sun_family = AF_UNIX;
        strcpy(client_addr.sun_path, name);

        if(bind(server_fd, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0)exit_error("Binding failed.");
        if(connect(server_fd, (struct sockaddr*) &loc_addr, sizeof(loc_addr)) < 0) exit_error("Connect to server failed.");
    }
    else if(connection_type == NET){
        if(port_no == NULL)exit_error("Port number is needed if connection method is net\n");
        
        net_addr.sin_family = AF_INET;
        net_addr.sin_port = htons(atoi(port_no));
        net_addr.sin_addr.s_addr = inet_addr(server_addr);
        if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) exit_error("Socket to server failed.");

        struct sockaddr_in client_addr;
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = inet_addr(server_addr);
        if(bind(server_fd, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0) exit_error("Binding failed.");
        if(connect(server_fd, (struct sockaddr*) &net_addr, sizeof(net_addr)) < 0) exit_error("Connect to server failed.");
    }
    else{
        exit_error("Invalid connection method\n");
    }

    printf("Proceeding to send initialazing message\n");
    char init_msg[MAX_MSG_LEN];
    sprintf(init_msg, "%d %s",CONNECT, name);
    if(send(server_fd, init_msg, MAX_MSG_LEN, 0) == -1) exit_error("Cannot send init msg");
    if(recv(server_fd, init_msg, MAX_MSG_LEN, 0) == -1) exit_error("Cannot receive message");
    int response;
    sscanf(init_msg, "%d", &response);
    if(response == REFUSAL){
        printf("%s\n", init_msg);
        exit(0);
    }else if(response == APPROVAL){
        printf("Succesully connected to server\n");    
    }
    else{
        printf("Incorrect response\n");
        exit(0);
    }

}

void ping_response(){
    char buf[MAX_MSG_LEN];
    sprintf(buf, "%d %s", PING, name);
    send(server_fd, buf, MAX_MSG_LEN, 0);

}

void stop_client(){
    printf("Shutting down\n");
    unlink(name);
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    exit(EXIT_SUCCESS);
}

void send_disconnect(){
    printf("Sending disconnect\n");
    char stop_msg[MAX_MSG_LEN];
    sprintf(stop_msg, "%d %s", DISCONNECT, name);
    send(server_fd, stop_msg, MAX_MSG_LEN, 0);
}

void sigint_handler_client(int sig, siginfo_t *sig_inf, void *ucontext){
    send_disconnect();
    exit(0);
}

void* check_msg(){
    char msg[MAX_MSG_LEN];
    int msg_code;
    while(1){
        recv(server_fd, msg, MAX_MSG_LEN, 0);
        sscanf(msg, "%d", &msg_code);
        if(msg_code == DISCONNECT) stop_client();
        if(msg_code == PING) ping_response();
    }
}

void make_move(char* board){
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, (void*) check_msg, NULL);

    print_board(board);
    int moved = 0, square = -1;
    while(!moved){
        printf("\nMake move by providing 1-9 number: ");
        scanf("%d", &square);
        if(board[square -1] != '.' || square > 9) printf("You've chosen taken square\n");
        else moved = 1;
    }
    board[square -1] = symbol;
    char msg[MAX_MSG_LEN];
    sprintf(msg, "%d %s %s", MOVE, name, board);
    send(server_fd, msg, MAX_MSG_LEN, 0);
    printf("Wait for your opponent's move\n");
    pthread_cancel(receive_thread);
    pthread_join(receive_thread, NULL);
}

int main(int argc, char** argv){
    if(argc < 4){
        printf("Insufficient number of arguments. Pass 4 or 5 arguments: name, connection method (local/net), server addres and port if connection method is net\n");
        return -1;
    }
    name = argv[1];
    connection_type = str_to_connection_type(argv[2]);
    server_addr = argv[3];
    atexit(stop_client);
    set_sigint_handling(sigint_handler_client);
    connect_to_server(argv[4]);
    char received_msg[MAX_MSG_LEN];
    sprintf(received_msg, "xd");
    int msg_code;
    char content[MAX_MSG_LEN];
    char board[9];
    char winner;
    while(1){
        recv(server_fd, received_msg, MAX_MSG_LEN, 0);
        sscanf(received_msg, "%d", &msg_code);
        switch (msg_code)
        {
        case WAIT:
            printf("I have to wait for an opponent\n");
            break;
        case DISCONNECT:
            printf("Server or my opponent stopped working.\n");
            return 0;
        case PING:
            ping_response();
            break;            
        case MOVE:
            sscanf(received_msg, "%d %9c", &msg_code, board);
            make_move(board);
            break;
        case START_GAME:
            sscanf(received_msg, "%d %c %9c %[^\t\n]", &msg_code, &symbol, board, content);
            printf("%s\n", content);
            if(symbol == 'o') make_move(board);
            else printf("Wait for your opponent move\n");
            break;
        case END_GAME:
            printf("Game over\n");
            sscanf(received_msg, "%d %c %9c", &msg_code, &winner, board);
            print_board(board);
            if(symbol == winner) printf("YOU WON\n");
            else if(winner == '.') printf("DRAW\n");
            else printf("YOU LOST\n");
            return 0;
        default:
            return -1;
        }
        


    }

}