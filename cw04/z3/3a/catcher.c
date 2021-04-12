#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int CAUGHT_COUNT= 0;
char* MODE = NULL;

int SIG_START;
int SIG_STOP;

void send_back(pid_t sender_PID){

        if(!strcmp(MODE, "sigqueue")){
            union sigval value;
            
            for (int i = 0; i < CAUGHT_COUNT; i++){
                value.sival_int = i;
                sigqueue(sender_PID, SIG_START, value);
            }
            value.sival_int = CAUGHT_COUNT;
            sigqueue(sender_PID, SIG_STOP, value);
        }
        else{
            for (int i=0; i < CAUGHT_COUNT; i++) kill(sender_PID, SIG_START);
            kill(sender_PID, SIG_STOP);
        }
        
    
}
void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    if (sig == SIG_START)    CAUGHT_COUNT++;
    else{
        printf("+Catcher Handler+\nI've received %d signals %s. Sending them back.\n\n", CAUGHT_COUNT, strsignal(SIG_START));
        send_back(sig_info->si_pid);
        printf("\nCATCHER EXCIT\n");
        exit(0);
    }
}


int main(int argc, char** argv){
    if (argc != 2){
        printf("Too few arguments\n");
        return -1;
    }

    MODE = argv[1];


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

    printf("Catcher PID:  %d\n", getpid());
    while(1){
        sleep(1);
    }

    
    return 0;
}