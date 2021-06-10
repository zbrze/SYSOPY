#include "header.h"
int local_socket;
int inet_socket;
int port_no;
int ready_to_play = -1;
char *socket_path;
pthread_t ping_thread, socket_thread;
client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; 


void stop_server(){
    pthread_cancel(socket_thread);
    char stop_msg[MAX_MSG_LEN];
    sprintf(stop_msg, "%d", DISCONNECT);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].fd != -1){
            send(clients[i].fd, stop_msg, MAX_MSG_LEN, 0);
        }
    }
    close(inet_socket);
    close(local_socket);
	unlink(socket_path);
}

int get_client(char* name){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(!strcmp(name, clients[i].name)) return i;
    }
    return -1;
}

int get_free_idx(){
    for (int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].fd == -1) return i;
    }
    return -1;
}

void start_server(){

    struct sockaddr local_socket_addr;
    struct sockaddr_in inet_socket_addr;
    if((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) exit_error("Unable to create local socket");
    local_socket_addr.sa_family = AF_UNIX;

	strcpy(local_socket_addr.sa_data, socket_path);
    if(bind(local_socket, &local_socket_addr, sizeof(local_socket_addr)) != 0)  exit_error("Unable to bind local");
    if(listen(local_socket, MAX_CLIENTS) != 0)exit_error("Unable to listen on local socket");

    printf("fd : %d\n", local_socket);

    if((inet_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) exit_error("Unable to create inet socket");
    inet_socket_addr.sin_family = AF_INET;
    inet_socket_addr.sin_port = htons(port_no);
    inet_socket_addr.sin_addr.s_addr =  inet_addr(IPv4_ADDRESS);
    
    if(bind(inet_socket, (struct sockaddr*) &inet_socket_addr, sizeof(inet_socket_addr)) != 0)  exit_error("Unable to bind inet");
    if(listen(inet_socket, MAX_CLIENTS) != 0)exit_error("Unable to listen on inet socket");

    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[i].fd = -1;
        clients[i].opponent = -1;
        clients[i].to_delete = 0;
        clients[i].symbol = '!';
        clients[i].game = NULL;

    }
}

void start_game(int idx_1, int idx_2){
     printf("Starting game: %s vs %s\n", clients[idx_2].name, clients[idx_1].name);
       
    clients[idx_1].opponent = idx_2;
    clients[idx_2].opponent = idx_1;
    gameplay *game = new_game();
    
    clients[idx_1].game = game;
    clients[idx_2].game = game;
    srand(time(NULL));
    if((rand() & 1) == 1){
        clients[idx_1].symbol = 'o';
        clients[idx_2].symbol = 'x';
    }else{
        clients[idx_1].symbol = 'x';
        clients[idx_2].symbol = 'o';
    }

    char msg[MAX_MSG_LEN];
    sprintf(msg, "%d %c %s New game with player %s. Your symbol is %c.", START_GAME, clients[idx_1].symbol, game->board,  clients[idx_2].name, clients[idx_1].symbol);
    send(clients[idx_1].fd, msg, MAX_MSG_LEN, 0);
    sprintf(msg, "%d %c %s New game with player %s. Your symbol is %c.", START_GAME,  clients[idx_2].symbol, game->board, clients[idx_1].name, clients[idx_2].symbol);
    send(clients[idx_2].fd, msg, MAX_MSG_LEN, 0);
}

void new_client(int fd){
    int client_fd = accept(fd, NULL, NULL);
    char name_msg[MAX_MSG_LEN];
    recv(client_fd, name_msg, MAX_MSG_LEN, 0);
    printf("Adding new client %s\n", name_msg);

    char reply[MAX_MSG_LEN];
    int client_idx = get_free_idx();
    int failed = 0;
    if(get_client(name_msg) != -1){
        printf("Client with this name already exists\n");
        sprintf(reply, "%d Client with this name already exists", REFUSAL);
        failed = 1;
    }else{
        if(client_idx == -1){
            printf("Server is full\n");
            sprintf(reply, "%d Server is full", REFUSAL);
            failed = 1;
        }
        else{
            strcpy(clients[client_idx].name, name_msg);
            clients[client_idx].fd = client_fd;
            clients[client_idx].to_delete = 0;
            sprintf(reply, "%d", APPROVAL);
        }
    }
    if(send(client_fd, reply, MAX_MSG_LEN, 0) == -1) exit_error("Cannot send reply"); 
    if(failed == 1) return;
   /* for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].fd != -1){
            printf("Client %s, fd: %d\n", clients[i].name, clients[i].fd);
        }
    }*/
    if(ready_to_play == -1){
        ready_to_play = client_idx;
        printf("Nobody is waiting to play\n");
        sprintf(reply, "%d", WAIT);
        if(send(client_fd, reply, MAX_MSG_LEN, 0) == -1) exit_error("Cannot send reply"); 
    }else{
        int idx_2 = ready_to_play;
        ready_to_play = -1;
        start_game(idx_2, client_idx);
    }
    return;
}

void disconnect_client(int index){
    printf("Disconnecting client %s\n", clients[index].name);

    if(clients[index].fd == -1){
        printf("Smth wrong, client not connected");
        return;
    }
    
    int opponent;
    if((opponent = clients[index].opponent) != -1){
        printf("Client %s is currently in game with %s\n", clients[index].name, clients[opponent].name);
        clients[opponent].opponent = -1;
        if(clients[opponent].fd != -1){ 
                char stop_msg[MAX_MSG_LEN];
                sprintf(stop_msg, "%d", DISCONNECT);
                send(clients[opponent].fd, stop_msg, MAX_MSG_LEN, 0);
                //disconnect_client(opponent);
            }
    }
    if(ready_to_play == index) ready_to_play = -1;
    shutdown(clients[index].fd, SHUT_RDWR);
    close(clients[index].fd);
    clients[index].fd = -1;
    strcpy(clients[index].name, "\0");
    clients[index].to_delete = 0;
    clients[index].symbol = '!';
    clients[index].opponent = -1;

    printf("Client succesfully disconnected\n");
}

char check_winner(char* board){

    if(board[0] == board[1] && board[1] == board[2]) return board[2];
    if(board[3] == board[4] && board[4] == board[5]) return board[5];
    if(board[6] == board[7] && board[7] == board[8]) return board[8];

    if(board[0] == board[3] && board[3] == board[6]) return board[6];
    if(board[1] == board[4] && board[4] == board[7]) return board[7];
    if(board[2] == board[5] && board[5] == board[8]) return board[8];

    if(board[0] == board[4] && board[4] == board[8]) return board[8];
    if(board[2] == board[4] && board[4] == board[6]) return board[6];

    return '.';
}

void update_board(int index, char* msg){

    int opp_index = clients[index].opponent;
    printf("Updating game between %s and %s: ", clients[index].name, clients[opp_index].name);
    gameplay *game = clients[index].game;
    if(game == NULL){
        printf("This game doesnt exist");
        return;
    }
    char board[9];
    int x;
    sscanf(msg, "%d %9s",&x, board);
    char winner = check_winner(board);
    if((winner == 'o' || winner == 'x')){
        char end_msg[MAX_MSG_LEN];
        printf("Game is over and the winner is %c\n", winner);
        sprintf(end_msg, "%d %c %s", END_GAME, winner, board);
        send(clients[index].fd, end_msg, MAX_MSG_LEN, 0);
        send(clients[opp_index].fd, end_msg, MAX_MSG_LEN, 0);
        disconnect_client(index);
    }
    else{
        game->board = board;
        game->turn_no++;
        printf("%d turn\n", game->turn_no);
        if(game->turn_no == 9){
            printf("Game is over and there's no winner\n");
            char end_msg[MAX_MSG_LEN];
            sprintf(end_msg, "%d %c %s", END_GAME, '.', board);
            send(clients[index].fd, end_msg, MAX_MSG_LEN, 0);
            send(clients[opp_index].fd, end_msg, MAX_MSG_LEN, 0);
            disconnect_client(index);
        }
        else{
            char reply_msg[MAX_MSG_LEN];
            sprintf(reply_msg, "%d %s", MOVE, board);
            send(clients[opp_index].fd, reply_msg, MAX_MSG_LEN, 0);  

        }
    }
     

}

void *socket_service(){
    struct pollfd client_descriptors[MAX_CLIENTS];
    struct pollfd server_descriptors[2];
    
    while(1){
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if(clients[i].name != NULL){
                client_descriptors[i].fd = clients[i].fd;
                client_descriptors[i].events = POLLIN;
                client_descriptors[i].revents = 0;
            }
        }
        
        server_descriptors[0].fd = local_socket;
        server_descriptors[1].fd = inet_socket;    
        server_descriptors[1].events = server_descriptors[0].events = POLLIN;

        //if(poll(client_descriptors, MAX_CLIENTS, -1) == -1) error_exit("Cannot poll");
        if(poll(server_descriptors, 2, 1) == -1) exit_error("Cannot poll");

        if(POLLIN){
            if(server_descriptors[0].revents) new_client(server_descriptors[0].fd);
            if(server_descriptors[1].revents) new_client(server_descriptors[1].fd);
        }
        if(poll(client_descriptors, MAX_CLIENTS, 1) == -1) exit_error("Cannot poll");
        
        for(int i = 0; i < MAX_CLIENTS; i++){
            char msg[MAX_MSG_LEN];
            int msg_code;
            if(clients[i].fd != -1 && client_descriptors[i].revents && POLLIN){
                
                recv(clients[i].fd, msg, MAX_MSG_LEN, 0);
                sscanf(msg, "%d", &msg_code);
                switch (msg_code)
                {
                case DISCONNECT:
                    disconnect_client(i);
                    break;
                case MOVE:
                    update_board(i, msg);
                case PING:
                    clients[i].to_delete = 0;
                    break;
                default:
                    break;
                }
            }

        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

void ping_service(){
    while(1){
        sleep(PING_CYCLE);
        pthread_mutex_lock(&clients_mutex);
        printf("Pinging service started\n");
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i].fd != -1){
                clients[i].to_delete = 1;
                char buf[MAX_MSG_LEN];
                sprintf(buf, "%d", PING);
                send(clients[i].fd, buf, MAX_MSG_LEN, 0);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(PING_CYCLE);
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i].to_delete == 1 ){
                printf("No ping response from: %s\n", clients[i].name);
                disconnect_client(i);
                }
        }
        pthread_mutex_unlock(&clients_mutex);
        printf("Pinging service ended\n");

    }
}

int main(int argc, char** argv){
    if(argc != 3){
        printf("Insufficient number of arguments. Pass two arguments: TCP port no and socket path\n");
        return -1;
    }
    port_no = atoi(argv[1]);
    socket_path = argv[2];
    atexit(stop_server);
    set_sigint_handling(sigint_handler);


    start_server();
    pthread_create(&socket_thread, NULL, (void*) socket_service, NULL);
    pthread_create(&ping_thread, NULL, (void*) ping_service, NULL);
    pthread_join(socket_thread, NULL);
}