#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
int SIG_1, SIG_2;
int sigusr1_counter = 0;
char* MODE;
void handler_sigusr1(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("Ive catched: %s\n", strsignal(sig));
    sigusr1_counter++;
}

int send_sigusrs_back(int sender_pid){

    int sig_usr1_sent = 0;
    if(!strcmp(MODE, "sigqueue")){
        for(int i = 0; i < sigusr1_counter; i++){ 
            union sigval value;
            value.sival_int = i;
            if(!sigqueue( sender_pid, SIGUSR1, value)) sig_usr1_sent++;
            sleep(0.2);
        }
        sleep(2);
        if(sigusr1_counter == sig_usr1_sent){ 
            union sigval value;
            value.sival_int = sigusr1_counter;
            if(!sigqueue(sender_pid, SIGUSR2, value)) return 0;
            return 0;
        }
    }else{
        for(int i = 0; i < sigusr1_counter; i++){ 
            if(!kill(sender_pid, SIG_1)) sig_usr1_sent++; 
            sleep(0.5);
            }
         sleep(2);
         if(sigusr1_counter == sig_usr1_sent){
            if(!kill(sender_pid, SIG_2)) return 0;
         }
    }
    
    return -1;
}

void handler_sigusr2(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("\nHANDLER STOP:");
    printf("\nIve received %d signals from ", sigusr1_counter);
    printf("PID: %d\n",sig_inf->si_pid);
    fflush(stdout);
    sleep(3);
    if(!send_sigusrs_back(sig_inf->si_pid)){
        _exit(0);
    }
    


}
void catching(){
    while (1)
    {
       sleep(1);
    }
    

}

int main(int argc, char** argv){
    MODE = argv[1];
    if(!strcmp(MODE, "sigrt")){
        SIG_1 = SIGRTMIN;
        SIG_2 = SIGRTMIN + 1;
    }else{
        SIG_1 = SIGUSR1;
        SIG_2 = SIGUSR2;
    }
    struct sigaction action;
    action.sa_sigaction = handler_sigusr1;
    action.sa_flags = SA_SIGINFO;
    action.sa_flags = SA_NODEFER;
    sigemptyset(&action.sa_mask);
    sigaction(SIG_1, &action, NULL);
    
    action.sa_sigaction = handler_sigusr2;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIG_2, &action, NULL);
    printf("%d\n\n", getpid());
    fflush(stdout);
    catching();
}