#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int sigusr1_counter = 0;
void handler_sigusr1(int sig){
    printf("Ive catched: %s\n", strsignal(sig));
    sigusr1_counter++;
}

void send_sigusrs_back(int sender_pid){
    int sig_usr1_sent = 0;
    for(int i = 0; i < sigusr1_counter; i++){ 
         if(!kill(sender_pid, SIGUSR1)) sig_usr1_sent++;
         sleep(0.2);
         }
         sleep(3);
         if(sigusr1_counter == sig_usr1_sent) kill(sender_pid, SIGUSR2);
}

void handler_sigusr2(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("HANDLER USR2:\n");
    printf("\nIve received %d SIGUSR1 signals ", sigusr1_counter);
    printf("PID: %d",sig_inf->si_pid);
    fflush(stdout);
    sleep(2.5);
    kill(sig_inf->si_pid, SIGUSR1);
    

    _exit(0);

}
void catching(){
    while (1)
    {
       sleep(1);
    }
    

}

int main(int argc, char** argv){
    struct sigaction action;
    action.sa_handler = &handler_sigusr1;
    action.sa_flags = SA_NODEFER;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR1, &action, NULL);
    
    action.sa_sigaction = handler_sigusr2;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR2, &action, NULL);
    printf("%d", getpid());
    fflush(stdout);
    catching();
}