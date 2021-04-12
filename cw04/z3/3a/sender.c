#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int sigusr1_counter = 0;
int got_usr2 = 0;
int sent_usr2 = 0;

void start_catching(){
    printf("Start catching started\n");
    while(1){
        sleep(0.5);
    }
}


void handler_sigusr1(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("Ive catched: %s\n", strsignal(sig)); 
    
}
void handler_sigusr2(int sig){
    printf("Ive catched: %s - sending ended\n", strsignal(sig));
    got_usr2 = 1;
}
int send_kill(pid_t cather, int sig_number){
     for(int i = 0; i < sig_number; i++){ 
         if(!kill(cather, SIGUSR1)) sigusr1_counter++;
         sleep(0.2);
         }
    sleep(3);
    if(sigusr1_counter == sig_number){ 
        if(!kill(cather, SIGUSR2)) sent_usr2 = 1;
        return 0;
    }
    return -1;
}




void send_sigrt(pid_t cather, int sig_number){
    for(int i = 0; i < sig_number; i++) kill(cather, SIGUSR1);
}

void send_sigqueue(pid_t cather, int sig_number){
    union sigval value;
    for(int i = 0; i < sig_number; i++){ 
         if(!sigqueue(cather, SIGUSR1, value)) sigusr1_counter++;
         sleep(0.2);
         }
         sleep(1);
         if(sigusr1_counter == sig_number) sigqueue(cather, SIGUSR2, value);
        sleep(2);
}

int main(int argc, char** argv){
    //argv[1] = how many, argv[2] = which send mode (kill, sigrt, sigqueue), argv[3] = catcher pid
    if(argc != 4){
        printf("Too few arguments");
        return -1;
    }
    struct sigaction action;
    sigemptyset(&action.sa_mask);

    action.sa_sigaction = handler_sigusr1;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &action, NULL);

    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR2, &action, NULL);

    int sig_num = atoi(argv[1]);
    pid_t catcher_pid = atoi(argv[3]);
    int res = 10;
    if(!strcmp(argv[2], "kill")) res = send_kill(catcher_pid, sig_num);
    else if(!strcmp(argv[2], "sigrt")) send_sigrt(catcher_pid, sig_num);
    else if(!strcmp(argv[2], "sigqueue")) send_sigqueue(catcher_pid, sig_num);
    else{ 
        printf("wrong sending mode");
        return -1;
        }
    if(!res){
        printf("start catching:\n");
        fflush(stdout);
        while(1){
            sleep(2);
        }
    }

    if(sigusr1_counter == sig_num) return 0;
    return -1;
}