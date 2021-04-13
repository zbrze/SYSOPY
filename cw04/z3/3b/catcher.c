#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int caught_signals = 0;
char* sending_mode = NULL;
int sending = 0;
int sent_back = 0;
int SIG_COUNT;
int SIG_END;
void send_notice(pid_t sender_PID){
    //printf("Sending notice\n");
    if(!strcmp(sending_mode, "sigqueue")){
            union sigval value;
            sigqueue(sender_PID, SIGUSR1, value);
        }
        else{
            kill(sender_PID, SIGUSR1);
        }
        
}

void send_stop(pid_t sender_PID){
    if(!strcmp(sending_mode, "sigqueue")){
            union sigval value;
            sigqueue(sender_PID, SIG_END, value);
        }
        else{
            kill(sender_PID, SIG_END);
        }
}

void send_back(pid_t sender_PID){
    if(!strcmp(sending_mode, "sigqueue")){
            union sigval value;
            sigqueue(sender_PID, SIG_COUNT, value);
        }
        else{
            kill(sender_PID, SIG_COUNT);
        }
        sent_back++;
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    printf("IVe received %d %s\n",  caught_signals, strsignal(sig));
    if(sending == 0){    
        if (sig == SIG_COUNT || sig == SIGUSR1){      
            caught_signals++;
            send_notice( sig_info -> si_pid);
        }
        else{          
            send_stop(sig_info -> si_pid);
            printf("Catcher: Received %d sig .\n\n", caught_signals);
            sending = 1;
            
            printf("CATCHER: im in sending mode:\n");
            //tu powinienem cos wyslac
            //exit(0);
        }
    }
    else{
        if (sig == SIG_COUNT || sig == SIGUSR1){  
            if(sent_back < caught_signals)    {

                send_back(sig_info -> si_pid);
                printf("CATCHER:ive sent %d signals back:\n", sent_back);
                fflush(stdout);
            }
            else{
                printf("CATCHER:Sending stop\n");
                send_stop(sig_info -> si_pid);
            }
        }else{
            printf("catcher ends working");
            exit(0);
        }
        
    }
}

int main(int argc, char** argv){
    
    if (argc != 2){
        printf("Too few arguments\n");
        return -1;
    }

    sending_mode = argv[1];


    sigset_t mask;          
    sigfillset(&mask);

    if (!strcmp(sending_mode, "sigrt")){
        sigdelset(&mask, SIGRTMIN + 1);
        sigdelset(&mask, SIGRTMIN + 2);
        sigdelset(&mask, SIGUSR1);
        SIG_COUNT = SIGRTMIN + 1;
        SIG_END = SIGRTMIN + 2;
    }
    else{
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        SIG_COUNT = SIGUSR1;
        SIG_END = SIGUSR2;
    }

    sigprocmask(SIG_SETMASK, &mask, NULL);

    struct sigaction action;
    sigemptyset(&action.sa_mask);

    sigaddset(&action.sa_mask, SIG_COUNT);
    sigaddset(&action.sa_mask, SIG_END);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;  

    if (!strcmp(sending_mode, "sigrt")){
         sigaddset(&action.sa_mask,SIGUSR1);
         sigaction(SIGUSR1, &action, NULL);
    }
    sigaction(SIG_COUNT, &action, NULL);
    sigaction(SIG_END, &action, NULL);

    printf("Catcher: Moj PID: %d\n", getpid());
    while (1){
        pause();
    }

    
    return 0;
}