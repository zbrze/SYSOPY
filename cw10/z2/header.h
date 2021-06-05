#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#define MAX_CLIENTS 13
#define MAX_MSG_LEN 250
#define MAX_NAME_LEN 10
#define IPv4_ADDRESS "127.0.0.1"
#define LOCAL 1
#define NET 2
#define PING_CYCLE 6
typedef enum msg_type{
    CONNECT = 1,
    DISCONNECT = 2,
    MOVE = 3,
    REFUSAL = 3,
    APPROVAL = 4,
    WAIT = 5,
    PING = 6,
    START_GAME = 7,
    END_GAME = 8
}msg_type; 


typedef struct gameplay{
    char *board;
    int turn_no;
}gameplay;

typedef struct client{
    char name[MAX_NAME_LEN];
    int fd;
    struct sockaddr *addr;
    int opponent;
    char symbol;
    int to_delete;
    gameplay *game;
}client;


int str_to_connection_type(char* str);
void exit_error(char *content);
void set_sigint_handling(void (*handler)(int,  siginfo_t *, void *));
void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext);
gameplay* new_game();
void print_board(char* board);