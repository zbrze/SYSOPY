#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>


char* MODE = NULL;

int RECEIVED_COUNT = 0;
int ALL_SIG_NUM = 0;

int SIG_START;
int SIG_STOP;
pid_t CATCHER_PID = 0; 

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    if (sig == SIG_STOP){
        printf("*Sender Handler*: I've received %d signals, while I've sent %d.\n",RECEIVED_COUNT, ALL_SIG_NUM);
        printf("SENDER EXIT\n");
        exit(0);
    }
    RECEIVED_COUNT++;
    if (!strcmp(MODE, "sigqueue")) printf("*Sender Handler*: I've received signal of index: %d.\n",sig_info -> si_value.sival_int);
    
}

void send_sig(){
    if (!strcmp(MODE, "sigqueue")){
        union sigval value;
        for (int i=0; i < ALL_SIG_NUM; i++){
            value.sival_int = i;
             sigqueue(CATCHER_PID, SIG_START, value);
        }
        sigqueue(CATCHER_PID, SIG_STOP, value);
    }
    else{
        for (int i=0; i < ALL_SIG_NUM; i++) kill(CATCHER_PID, SIG_START);
        kill(CATCHER_PID, SIG_STOP);
    }
    
}

int main(int argc, char** argv){
    //argv[1] = how many, argv[2] = which send mode (kill, sigrt, sigqueue), argv[3] = catcher pid
    if(argc != 4){
        printf("Too few arguments\n");
        return -1;
    }
    printf("I'm a sender!\n");
    MODE = argv[2];
    CATCHER_PID = atoi(argv[3]);
    ALL_SIG_NUM = atoi(argv[1]);
    sigset_t mask;   
    sigfillset(&mask);


    if (!strcmp(MODE, "sigrt")){
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMIN + 1);
        SIG_START = SIGRTMIN;
        SIG_STOP = SIGRTMIN + 1;
    }
    else{
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        SIG_START = SIGUSR1;
        SIG_STOP = SIGUSR2;
    }
    
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIG_START);
    sigaddset(&action.sa_mask, SIG_STOP);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;  
    sigaction(SIG_START, &action, NULL);
    sigaction(SIG_STOP, &action, NULL);

    send_sig();
     
    while(1){
        sleep(1);
    }
    
    return 0;
}
