#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

int received_signals = 0;
char* glob_mode = NULL;
int signal_count;
int catching = 0;
int SIG_COUNT;
int SIG_END;

void send_sig(pid_t catcher_PID){
    printf("SENDER %d SENDING SIG\n", received_signals);
   if (!strcmp(glob_mode, "sigqueue")){
        union sigval value;
        sigqueue(catcher_PID, SIG_COUNT, value);
    }
    else{
        kill(catcher_PID, SIG_COUNT);
    }
}

void send_stop(pid_t catcher_PID){
   if (!strcmp(glob_mode, "sigqueue")){
        union sigval value;
        sigqueue(catcher_PID, SIG_END, value);
    }
    else{
        kill(catcher_PID, SIG_END);
    }
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    pid_t catcher_PID = sig_info -> si_pid;
   
    if (sig == SIGUSR1 || sig == SIG_COUNT){
        if(received_signals <= signal_count){   

            received_signals++;
            if (received_signals < signal_count) send_sig(catcher_PID);
            else{     
                send_stop(catcher_PID);
            }   
        }else{
            printf("SENDER: Ive received one signal back");
            fflush(stdout);
        }
    
    }
        else{
            //waiting for back
            printf("Sender: Dostalem %d sygnalow, powinno ich byc %d.\n\n",received_signals, signal_count);
            received_signals ++;
            send_sig(catcher_PID);
        }
    
}
int main(int argc, char** argv){

    if (argc != 4){
        fprintf(stderr, "Sender: Podano niepoprawna ilosc parametrow dla programu sender! Powinien byc ich 3!\n");
        return -1;
    }

    char* PID_chr = argv[3];
    char* sig_count_chr = argv[1];
    char* sending_mode = argv[2];

    
    pid_t catcher_PID = atoi(PID_chr);
    signal_count = atoi(sig_count_chr);

    

    glob_mode = sending_mode;

    sigset_t mask;          // Wypelnianie maski sygnalow tak, by odbierala tylko SIGUSR1 i SIGUSR2
    sigfillset(&mask);
    if (!strcmp(sending_mode, "sigrt")){
        sigdelset(&mask, SIGRTMIN + 2);
        sigdelset(&mask, SIGRTMIN + 1);
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


    send_sig(catcher_PID);
     
    while(1){
        pause();
    }
    
    return 0;
}