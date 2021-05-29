#include "header.h"

void exit_error(char *content){
    perror(content);
    exit(-1);
}

int str_to_connection_type(char* str){
    if(!strcasecmp(str, "local")) return LOCAL;
    if(!strcasecmp(str, "net")) return NET;
    return -1;
}

gameplay* new_game(){
    gameplay *game = calloc(1, sizeof(gameplay));
    game->board = calloc(9, sizeof(char));
    for(int i = 0; i < 9; i++) game->board[i] = '.';
    game->turn_no = 0;
    return game;
}

void set_sigint_handling(void (*handler)(int,  siginfo_t *, void *)){
    struct sigaction action;
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
}

void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("\nreceived sigint, going to stop\n");
    exit(0);
}

void print_board(char* board){
    printf("|-----|-----|-----|\n");
    for(int i = 0; i < 9; i++){
        printf("|  %c  ", board[i]);
        if((i + 1) % 3 == 0)
            printf("|\n|-----|-----|-----|\n");
    }
}