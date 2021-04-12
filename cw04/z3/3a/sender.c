#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int sigusr1_counter = 0;
int got_usr1 = 0;
int sent_usr2 = 0;
int SIG_1, SIG_2;
char* MODE;


void handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("Ive catched: %s\n", strsignal(sig)); 

    if(sig == 12 || sig == 35){
        printf("\nreceived signals: %d", got_usr1);
        exit(0);
    }
    got_usr1++;
    
}
void sigqueue_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("Ive catched: %s, of index: %d\n", strsignal(sig), sig_inf->si_value.sival_int); 
    if(sig == 12){
        printf("\nreceived signals: %d", got_usr1);
        exit(0);
    }
    got_usr1++;
    
}


int send_kill(pid_t cather, int sig_number){
     for(int i = 0; i < sig_number; i++){ 
         if(!kill(cather, SIGUSR1)) sigusr1_counter++;
         sleep(0.2);
         }
    sleep(2);
    if(sigusr1_counter == sig_number){ 
        if(!kill(cather, SIGUSR2)) sent_usr2 = 1;
        return 0;
    }
    return -1;
}


int send_sigrt(pid_t cather, int sig_number){
    for(int i = 0; i < sig_number; i++){ 
         if(!kill(cather, SIGRTMIN)) sigusr1_counter++;
         sleep(0.2);
         }
    sleep(2);
    if(sigusr1_counter == sig_number){ 
        if(!kill(cather, SIGRTMIN + 1)) sent_usr2 = 1;
        return 0;
    }
    return -1;
}

int send_sigqueue(pid_t cather, int sig_number){
    for(int i = 0; i < sig_number; i++){ 
        union sigval value;
        value.sival_int = i;
        if(!sigqueue(cather, SIGUSR1, value)) sigusr1_counter++;
         sleep(0.2);
         }
    sleep(2);
    if(sigusr1_counter == sig_number){ 
        union sigval value;
        value.sival_int = sig_number;
        if(!sigqueue(cather, SIGUSR2, value)) sent_usr2 = 1;
        return 0;
    }
    return -1;
}

int main(int argc, char** argv){
    //argv[1] = how many, argv[2] = which send mode (kill, sigrt, sigqueue), argv[3] = catcher pid
    if(argc != 4){
        printf("Too few arguments");
        return -1;
    }
    MODE = argv[2];


    if(!strcmp(MODE, "sigrt")){
        SIG_1 = SIGRTMIN;
        SIG_2 = SIGRTMIN + 1;
    }else{
        SIG_1 = SIGUSR1;
        SIG_2 = SIGUSR2;
    }

    struct sigaction action;
    sigemptyset(&action.sa_mask);

    if(!strcmp(MODE, "sigqueue")){
        action.sa_sigaction = sigqueue_handler;
    }else{
        action.sa_sigaction = handler;
    }
    
    action.sa_flags = SA_SIGINFO;
    sigaction(SIG_1, &action, NULL);

    sigemptyset(&action.sa_mask);
    sigaction(SIG_2, &action, NULL);

    int sig_num = atoi(argv[1]);
    pid_t catcher_pid = atoi(argv[3]);
    int res = 1;

    if(!strcmp(MODE, "kill")) res = send_kill(catcher_pid, sig_num);
    else if(!strcmp(MODE, "sigrt")) res = send_sigrt(catcher_pid, sig_num);
    else if(!strcmp(MODE, "sigqueue")) res = send_sigqueue(catcher_pid, sig_num);
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