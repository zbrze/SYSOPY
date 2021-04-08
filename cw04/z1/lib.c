#include "lib.h"
#include<stdio.h>
#include<string.h>
#include<signal.h>
#include <sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include <stddef.h>


void handler(int sig){
    printf("Process %d has received signal: %s \n", getpid(),  strsignal(sig));
}

void check_if_ignoring(int sig){
    struct sigaction sa;
    sigaction(sig, NULL, &sa);
    sa.sa_handler == SIG_IGN ? printf("Process %d is ignoring %s\n", getpid(), strsignal(sig)) :  printf("Process %d is NOT ignoring SIGUSR1\n", getpid());

}
void ignore(int sig){
    printf("\n----------------------TEST IGNORING:----------------------\n");
    signal(sig, handler);
    signal(sig, SIG_IGN);
    printf("Father process:\n");
    raise(sig);
    check_if_ignoring(sig);
}

void handler_testing(int sig){    
    printf("\n----------------------TEST HANDLING:----------------------\n");

    signal(SIGUSR1, handler);
    printf("Father process: ");
    raise(SIGUSR1);
}
void check_if_waiting(int sig){
    sigset_t pnd;
    sigpending(&pnd);
    sigismember(&pnd, sig) ? printf("Signal %s is waiting in process: %d\n",  strsignal(sig), getpid()) : printf("Signal %s is NOT waiting in process: %d\n", strsignal(sig), getpid()) ;
}

void mask(int sig){
    printf("\n----------------------TEST MASKING:----------------------\n");

    sigset_t new_mask, old_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, sig);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);  
    raise(sig);
    check_if_waiting(sig);
}

void pending(int sig){
    printf("\n----------------------TEST PENDING:----------------------\n");

    sigset_t new_mask, old_mask;
    sigemptyset(&new_mask);
    sigaddset(&new_mask, sig);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);  
    raise(sig);
    printf("Parent process: \n");
    check_if_waiting(sig);
    wait(NULL);
}
