
#define _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include<assert.h>
#include<ucontext.h>
#include <sys/types.h>
#include<time.h>
#include <sys/syscall.h>
#include<errno.h>

void basic_handler_print(siginfo_t *sig_inf){
    printf("\n*Handler info:*");
    printf("\nNumber of signal: %d\nPID of sending process: %d\nReal user id of sending process: %d\nExit value: %d\nSignal code: %d\nMeaning: ", \
    sig_inf->si_signo, sig_inf->si_pid, sig_inf->si_uid, sig_inf->si_status, sig_inf->si_code);
    
}

void handler_sigsegv(int sig, siginfo_t *sig_inf, void *ucontext){
   basic_handler_print(sig_inf);
   switch(sig_inf->si_code) {
        case 1: printf("SEGV_MAPERR\n"); break;
        case 2: printf("SEGV_ACCERR\n"); break;
        default: printf("Unknow code\n"); break;
    }
    printf("\n");
    
    
    _exit(0);
}

void handler_sigabrt(int sig, siginfo_t *sig_inf, void *ucontext){
    basic_handler_print(sig_inf);
    _exit(0);
}
void handler_sigfpe(int sig, siginfo_t *sig_inf, void *ucontext){
    basic_handler_print(sig_inf);
    _exit(0);
}

void call_SIGSEGV(){
    printf("\n\n========CALLING SIGSEGV========\n\n");
    struct sigaction action;
    action.sa_sigaction = handler_sigsegv;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGSEGV, &action, NULL);
    if (fork() == 0){
        printf("--Calling SIGSEGC by assigning an string to pointer value:--\n");
        printf("PID according to getpid(): %d", getpid());
       char* str = "foo";
       *str = "chhs";
    }
    wait(NULL);
    
    if(fork() == 0){
        printf("\n--Calling SIGSEGC by reference to null value:--\n");

        printf("\nPID according to getpid(): %d\n", getpid());
        int* ptr = NULL;
        ptr[20] = 11;
    }
    wait(NULL);
    
     if(fork() == 0){
         printf("\n--Calling SIGSEGC by raise() function:--\n");
        printf("\nPID according to getpid(): %d\n", getpid());
         raise(SIGSEGV);
     }
     wait(NULL);
}

void call_SIGABRT(){
    printf("\n\n========CALLING SIGABRT========\n\n");
    struct sigaction action;
    action.sa_sigaction = handler_sigabrt;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGABRT, &action, NULL);
    printf("--Calling SIGABRT by assertion:--\n");
    if(fork() == 0){
        printf("\nPID according to getpid(): %d\n", getpid());
        assert(1==0);
    }
    wait(NULL);
}

void call_SIGFPE(){
    printf("\n\n========CALLING SIGFPE========\n\n");
   
    struct sigaction action;
    action.sa_sigaction = handler_sigfpe;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGFPE, &action, NULL);
    
    if(fork() == 0){
        printf("--Calling SIGFPE by dividing by 0--:\n");
        printf("\nPID according to getpid(): %d\n", getpid());
        int zero = 0;
        printf("%d", 1/zero); 
    }
    wait(NULL);

    if(fork() == 0){
        printf("\n\n--Calling SIGFPE by raise() function:--\n");
        printf("\nPID according to getpid(): %d\n", getpid());
        raise(SIGFPE);
    }

    wait(NULL);
}


void handler_nocldstop(int sig){
    printf("\n*Handler  Info:*\nNumber of signal: %d\nPID of sending process: %d\n", sig, getpid());
}
void nocldstop(){
    struct sigaction action;
    action.sa_handler = &handler_nocldstop;

    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);
    printf("*Flag SA_NOCLDSTOP NOT set*\n");
    pid_t child;
    if((child = fork()) == 0){
        for(;;);
    }
    kill(child, SIGSTOP);
    sleep(2);


    action.sa_flags = SA_NOCLDSTOP;

    sigaction(SIGCHLD, &action, NULL);
    pid_t child2;
    printf("\n\n*Flag SA_NOCLDSTOP set*\n");
    printf("(No handler info should be printed)\n");
    if((child2 = fork()) == 0){
        for(;;);
    }
    kill(child2, SIGSTOP);
    sleep(1);
}

void handler_resethand(int sig){
    if(sig == 17) return;
    printf("\n*Handler Info:*\nNumber of signal: %d\nPID of sending process: %d\n", sig, getpid());
}

void resethand(){
    struct sigaction action;
    action.sa_handler = &handler_resethand;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGCHLD, &action, NULL);
    printf("=Flag SA_RESETHAND NOT set=\n");

    printf("(TWO SIGINT handler info should be printed)\n");
    pid_t child;
    if((child = fork()) == 0){
       raise(SIGINT);
       raise(SIGINT);
       _exit(0);
    }
    wait(NULL);
    printf("\n\n");
    
    action.sa_flags = SA_RESETHAND;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    printf("=Flag SA_RESETHAND set=\n");
    printf("(ONE SIGINT handler info should be printed)\n");
    if(fork() == 0){
        raise(SIGINT);
        raise(SIGINT);
    }
    wait(NULL);
}

int main(){
   printf("------------------SA_SIGINFO TEST-----------------\n");
    call_SIGSEGV();
    call_SIGABRT();
    call_SIGFPE();
     printf("\n\n\n------------------SA_NOCLDSTOP TEST-----------------\n");
    nocldstop();
     printf("\n\n\n------------------SA_RESETHAND TEST-----------------\n");
    resethand();
return 0;
    
}