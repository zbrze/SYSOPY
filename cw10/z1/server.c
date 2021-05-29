#include "header.h"
int local_socket;
int inet_socket;
int port_no;
int ready_to_play = -1;
char *socket_path;
struct sockaddr local_socket_addr;
struct sockaddr_in inet_socket_addr;

client clients[MAX_CLIENTS];

void stop_server(){
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
       // disconnect_client(opponent);
    }
    if(shutdown(clients[index].fd, SHUT_RDWR) == -1) exit_error("Unable to shutdown");
    if(close(clients[index].fd) == -1) exit_error("Unable to close client");
    clients[index].fd = -1;
    strcpy(clients[index].name, "\0");
    clients[index].to_delete = 0;
    clients[index].symbol = '!';
    clients[index].opponent = -1;

    printf("Client succesfully disconnected\n");
}

char check_winner(char* board){
   int count = 0;
    //sscanf(board, "%s", board_);
    for(int i = 0; i < 9; i++){
        if(board[i] =='x') count++;
        else if(board[i] =='o') count--;
        if(count == 3) return 'x';
        if(count == -3) return 'o';
        if(i % 3 == 2) count = 0;
    }
    for(int i = 0; i < 3; i++){
        
        for(int j = 0; j <= 9; j +=3){
            if(board[j + i] == 'x') count++;
            else if(board[j + i] =='o') count--;
            if(count == 3) return 'x';
            if(count == -3) return 'o';
        }
        count = 0;
    }
    if(board[0] != '.' && board[0] == board[4] && board[4] == board[8]) return board[8];
    if(board[2] != '.' && board[2] == board[4] && board[4] == board[6]) return board[6];
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
        if(clients[index].symbol == winner){
            printf("Game is over and the winner is %s", clients[index].name);
        }else{
            printf("Game is over and the winner is %s", clients[opp_index].name);
        }
    }
    game->board = board;
    game->turn_no++;
    printf("%d turn\n", game->turn_no);
    if(game->turn_no == 9){
        printf("Game is over and there's no winner\n");
    }
    char reply_msg[MAX_MSG_LEN];
    sprintf(reply_msg, "%d %s", MOVE, board);
    send(clients[opp_index].fd, reply_msg, MAX_MSG_LEN, 0);    

}

void socket_service(){
    struct pollfd client_descriptors[MAX_CLIENTS];
    struct pollfd server_descriptors[2];
    char msg[MAX_MSG_LEN];
    int msg_code;
    while(1){
        fflush(stdout);
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
    }
}

void ping_service(){}

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
    socket_service();
}