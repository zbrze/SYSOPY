#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

int received_signals = 0;
char* glob_mode = NULL;
int signal_count;

int SIG_COUNT;
int SIG_END;

void send_sig(pid_t catcher_PID){
   if (!strcmp(glob_mode, "sigqueue")){
        union sigval value;
        sigqueue(catcher_PID, SIG_COUNT, value);
    }
    else{
        kill(catcher_PID, SIG_COUNT);
    }
}

void send_SIG_END(pid_t catcher_PID){
    if ( strcmp(glob_mode, "kill") == 0 || strcmp(glob_mode, "sigrt") == 0 ){
        // kill lub sigrt
        if ( kill(catcher_PID, SIG_END) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu konczacego za pomoca kill!\n");
            exit(2);
        }
    }
    else{
        // sigqueue
        union sigval value;
        value.sival_int = 0;
        if ( sigqueue(catcher_PID, SIG_END, value) < 0 ){
            fprintf(stderr,"Sender: Nie wyslalem poprawnie sygnalu konczacego za pomoca sigqueue!\n");
            exit(3);
        }
    }
}

void signal_handler(int sig, siginfo_t* sig_info, void* ucontext){
    pid_t catcher_PID = sig_info -> si_pid;
    // printf("Sender: otrzymalem sygnal - %d\n", received_signals);        // Dzieki temu mozemy sprawdzic, czy sygnaly sa otrzymywane na zmiane
    if (sig == SIGUSR1){
        received_signals++;
        if (received_signals < signal_count){       // Jesli nie dostlismy tyle co trzeba to wysylamy nowe
            send_sig(catcher_PID);
        }
        else{       // Jak nie to wysylamy SIG_END
            send_SIG_END(catcher_PID);
        }   
    }
    else{
        printf("Sender: Dostalem %d sygnalow, powinno ich byc %d.\nSender: Koncze dzialanie.\n\n",received_signals, signal_count);
        exit(0);
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
