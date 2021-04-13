#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int caught_signals = 0;
int caught_notice = 0;
char* MODE = NULL;
int sending = 0;
int sent_back = 0;
int SIG_START;
int SIG_STOP;

void send_notice(pid_t sender_PID){
    //printf("Sending notice\n");
    if(!strcmp(MODE, "sigqueue")){
            union sigval value;
            sigqueue(sender_PID, SIGUSR1, value);
        }
        else{
            kill(sender_PID, SIGUSR1);
        }
        
}

void send_stop(pid_t sender_PID){
    if(!strcmp(MODE, "sigqueue")){
            union sigval value;

            value.sival_int = caught_signals;
            sigqueue(sender_PID, SIG_STOP, value);
        }
        else{
            kill(sender_PID, SIG_STOP);
        }
}

void send_back(pid_t sender_PID){
    if(!strcmp(MODE, "sigqueue")){
            union sigval value;
            value.sival_int = sent_back;
            sigqueue(sender_PID, SIG_START, value);
        }
        else{
            kill(sender_PID, SIG_START);
        }
        sent_back++;
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    //printf("IVe received %d %s\n",  caught_signals, strsignal(sig));
    if(sending == 0){    
        if (sig == SIG_START || sig == SIGUSR1){      
            caught_signals++;
            send_notice( sig_info -> si_pid);
        }
        else{          
            send_stop(sig_info -> si_pid);
            printf("\n+Catcher Handler+: Received %d sig.\nGonna start sending them back\n\n", caught_signals);
            sending = 1;
            
            //tu powinienem cos wyslac
            //exit(0);
        }
    }
    else{
        if (sig == SIG_START || sig == SIGUSR1){  
            if(sent_back < caught_signals)    {
                caught_notice++;
                send_back(sig_info -> si_pid);
                //printf("CATCHER:ive sent %d signals back:\n", sent_back);
                fflush(stdout);
            }
            else{
                
                send_stop(sig_info -> si_pid);
                printf("+Catcher Handler+: Ive received %d notice from sender\n\n", caught_notice);

            }
        }else{
            printf("+Catcher Handler+: all signals sent back to sender\nCatcher ends working\n");
            exit(0);
        }
        
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
        sigdelset(&mask, SIGRTMIN + 1);
        sigdelset(&mask, SIGRTMIN + 2);
        sigdelset(&mask, SIGUSR1);
        SIG_START = SIGRTMIN + 1;
        SIG_STOP = SIGRTMIN + 2;
    }
    else{
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        SIG_START = SIGUSR1;
        SIG_STOP = SIGUSR2;
    }

    sigprocmask(SIG_SETMASK, &mask, NULL);

    struct sigaction action;
    sigemptyset(&action.sa_mask);

    sigaddset(&action.sa_mask, SIG_START);
    sigaddset(&action.sa_mask, SIG_STOP);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;  

    if (!strcmp(MODE, "sigrt")){
         sigaddset(&action.sa_mask,SIGUSR1);
         sigaction(SIGUSR1, &action, NULL);
    }
    sigaction(SIG_START, &action, NULL);
    sigaction(SIG_STOP, &action, NULL);
    printf("Catcher PID:  %d\n", getpid());
    while (1){
        pause();
    }

    
    return 0;
}