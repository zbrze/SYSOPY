#include "header.h"
int local_socket;
int inet_socket;
int port_no;
int ready_to_play = -1;
char *socket_path;
pthread_t ping_thread, socket_thread;
client clients[MAX_CLIENTS];
struct sockaddr local_socket_addr;
struct sockaddr_in inet_socket_addr;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; 

void stop_server(){
    pthread_cancel(socket_thread);
    char stop_msg[MAX_MSG_LEN];
    sprintf(stop_msg, "%d", DISCONNECT);
    printf("Sending stop msgs:\n");
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].fd != -1){
            printf("sending stop to :%s\n", clients[i].name);
            sendto(clients[i].fd, stop_msg, MAX_MSG_LEN, 0, clients[i].addr, sizeof(struct sockaddr));
        }
    }
    fflush(stdout);
    close(inet_socket);
    close(local_socket);
	unlink(socket_path);
}

int get_client(char* name){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(!strcmp(name, clients[i].name)) {return i;}
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

    local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(local_socket == -1) exit_error("Cannot initalize local socket");
	local_socket_addr.sa_family = AF_UNIX;
	strcpy(local_socket_addr.sa_data, socket_path);
	if(bind(local_socket, &local_socket_addr, sizeof(local_socket_addr)) == -1) exit_error("Cannot bind");
	printf("Local socket fd: %d\n", local_socket);

   if((inet_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) exit_error("Unable to create inet socket");
    inet_socket_addr.sin_family = AF_INET;
    inet_socket_addr.sin_port = htons(port_no);
    inet_socket_addr.sin_addr.s_addr =  inet_addr(IPv4_ADDRESS);
    
    if(bind(inet_socket, (struct sockaddr*) &inet_socket_addr, sizeof(inet_socket_addr)) != 0)  exit_error("Unable to bind inet");

    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[i].fd = -1;
        clients[i].opponent = -1;
        clients[i].to_delete = 0;
        clients[i].symbol = '!';
        clients[i].game = NULL;
        clients[i].addr = NULL;
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
    if(sendto(clients[idx_1].fd, msg, MAX_MSG_LEN, 0, clients[idx_1].addr, sizeof(struct sockaddr)) == -1) perror("1");
    sprintf(msg, "%d %c %s New game with player %s. Your symbol is %c.", START_GAME,  clients[idx_2].symbol, game->board, clients[idx_1].name, clients[idx_2].symbol);
    if(sendto(clients[idx_2].fd, msg, MAX_MSG_LEN, 0, clients[idx_2].addr, sizeof(struct sockaddr)) == -1) perror("2");
}

void new_client(int fd, char* name, struct sockaddr* addr){
    
    printf("Adding new client %s\n", name);

    char reply[MAX_MSG_LEN];
    int client_idx = get_free_idx();
    int failed = 0;
    if(get_client(name) != -1){
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
            strcpy(clients[client_idx].name, name);
            clients[client_idx].fd = fd;
            clients[client_idx].addr = addr;
            clients[client_idx].to_delete = 0;
            sprintf(reply, "%d", APPROVAL);
            printf("new client: %s, %d\n", clients[client_idx].name,  clients[client_idx].fd);
        }
    }
    if(sendto(fd, reply, MAX_MSG_LEN, 0, (struct sockaddr *) addr, sizeof(struct sockaddr)) == -1) exit_error("Cannot send reply"); 
    if(failed == 1) return;
  /* 
    }*/
    if(ready_to_play == -1){
        ready_to_play = client_idx;
        printf("Nobody is waiting to play\n");
        sprintf(reply, "%d", WAIT);
        if(sendto(fd, reply, MAX_MSG_LEN, 0, (struct sockaddr *) addr, sizeof(struct sockaddr)) == -1) exit_error("Cannot send reply"); 
    }else{
        int idx_2 = ready_to_play;
        ready_to_play = -1;
        start_game(idx_2, client_idx);
    }
    return;
}

void disconnect_client(char* name){
    printf("Disconnecting client %s\n", name);
    int index = get_client(name);
    printf("%d %s\n", clients[index].fd, clients[index].name);
  
    if(index == -1 || clients[index].fd == -1){
        printf("Smth wrong, client not connected");
        return;
    }
    
    int opponent;
    if((opponent = clients[index].opponent) != -1){
        printf("opponents name %s\n",clients[opponent].name);
        printf("Client %s is currently in game with %s\n", clients[index].name, clients[opponent].name);
        clients[opponent].opponent = -1;
        char msg[MAX_MSG_LEN];
        sprintf(msg, "%d", DISCONNECT);
        sendto(clients[opponent].fd, msg, MAX_MSG_LEN, 0, clients[opponent].addr, sizeof(struct sockaddr));
        disconnect_client(clients[opponent].name);
    }else{
        ready_to_play = -1;
    }
   
    //shutdown(clients[index].fd, SHUT_RDWR);
    //close(clients[index].fd);

    clients[index].fd = -1;
    strcpy(clients[index].name, " ");
    clients[index].to_delete = 0;
    clients[index].symbol = '!';
    clients[index].opponent = -1;
    printf("Client succesfully disconnected\n");
    printf("%d %s\n", index, clients[index].name);
    
}

char check_winner(char* board){
   int count = 0;
    for(int i = 0; i < 9; i++){
        if(board[i] =='x') count++;
        else if(board[i] =='o') count--;
        if(count == 3) return 'x';
        if(count == -3) return 'o';
        if(i % 3 == 2) count = 0;
    }
    if(board[0] == board[3] && board[3] == board[6]) return board[6];
    if(board[1] == board[4] && board[4] == board[7]) return board[7];
    if(board[2] == board[5] && board[5] == board[8]) return board[8];

    if(board[0] != '.' && board[0] == board[4] && board[4] == board[8]) return board[8];
    if(board[2] != '.' && board[2] == board[4] && board[4] == board[6]) return board[6];
    return '.';
}

void update_board(char* name, char* board){

    int index = get_client(name);
    int opp_index = clients[index].opponent;
    printf("Updating game between %s and %s\n", clients[index].name, clients[opp_index].name);
    gameplay *game = clients[index].game;
    if(game == NULL){
        printf("This game doesnt exist");
        return;
    }
    char winner = check_winner(board);
    printf("WINNER: %c\n", winner);
    
    if((winner == 'o' || winner == 'x')){
        char end_msg[MAX_MSG_LEN];
        printf("Game is over and the winner is %c\n", winner);
        sprintf(end_msg, "%d %c %s", END_GAME, winner, board);
        sendto(clients[index].fd, end_msg, MAX_MSG_LEN, 0, clients[index].addr, sizeof(struct sockaddr));
        sendto(clients[opp_index].fd, end_msg, MAX_MSG_LEN, 0, clients[opp_index].addr, sizeof(struct sockaddr));
        return;
    }
    game->board = board;
    game->turn_no++;
    printf("%d turn %s move\n", game->turn_no, clients[opp_index].name);
    if(game->turn_no == 9){
        printf("Game is over and there's no winner\n");
        char end_msg[MAX_MSG_LEN];
        sprintf(end_msg, "%d %c %s", END_GAME, '.', board);
        sendto(clients[index].fd, end_msg, MAX_MSG_LEN, 0, clients[index].addr, sizeof(struct sockaddr));
        sendto(clients[opp_index].fd, end_msg, MAX_MSG_LEN, 0, clients[opp_index].addr, sizeof(struct sockaddr));
    }
    char reply_msg[MAX_MSG_LEN];
    sprintf(reply_msg, "%d %s", MOVE, board);
    sendto(clients[opp_index].fd, reply_msg, MAX_MSG_LEN, 0, clients[opp_index].addr, sizeof(struct sockaddr));    
    
}

void *socket_service(){
    struct pollfd server_descriptors[2];
    char msg[MAX_MSG_LEN];
    int msg_code;
    char msg_content[MAX_MSG_LEN];
    char name[MAX_NAME_LEN];
    while(1){
        fflush(stdout);

        pthread_mutex_lock(&clients_mutex);        
        server_descriptors[0].fd = local_socket;
        server_descriptors[1].fd = inet_socket;    
        server_descriptors[1].events = server_descriptors[0].events = POLLIN;

        //if(poll(client_descriptors, MAX_CLIENTS, -1) == -1) error_exit("Cannot poll");
        if(poll(server_descriptors, 2, 1) == -1) exit_error("Cannot poll");
      
        for(int i = 0; i < 2; i++){
            if(server_descriptors[i].revents && POLLIN){
                struct sockaddr* addr = calloc(1, sizeof(struct sockaddr));
				socklen_t addr_len = sizeof(&addr);
                recvfrom(server_descriptors[i].fd, msg, MAX_MSG_LEN, 0, addr, &addr_len);
                sscanf(msg, "%d %[^\t\n]", &msg_code, msg_content);
                switch (msg_code){
                    case CONNECT:
                        new_client(server_descriptors[i].fd, msg_content, addr);
                        break;
                    case DISCONNECT:
                        disconnect_client(msg_content);
                        break;
                    case MOVE:
                        sscanf(msg_content, "%s %9s", name, msg_content);
                        update_board(name, msg_content);
                        break;
                    case PING:
                        clients[get_client(msg_content)].to_delete = 0;
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
                sendto(clients[i].fd, buf, MAX_MSG_LEN, 0, clients[i].addr, sizeof(struct sockaddr));
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(PING_CYCLE);
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i].to_delete == 1 ){
                printf("No ping response from: %s\n", clients[i].name);
                disconnect_client(clients[i].name);
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