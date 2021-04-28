#include "header.h"

int rand_sleep(int min, int max){
    return ((rand() % (min - max) + min)*100000);
}

int rand_pizza(){
    return rand() % (min_pizza - max_pizza) + min_pizza;
}

char* get_time(){
  struct timeval tv;
  struct tm* ptm;
  char *time_string = (char*)calloc(30, sizeof(char));
  gettimeofday(&tv, NULL);
  ptm = localtime(&tv.tv_sec);
  strftime(time_string, 30, "%Y-%m-%d %H:%M:%S", ptm);
  long milliseconds = tv.tv_usec / 1000;
  sprintf(time_string,"%s:%03ld", time_string, milliseconds);
  return time_string;
}

void exit_error(char *content){
    perror(content);
    exit(-1);
}
void set_sigint_handling(void (*handler)(int,  siginfo_t *, void *)){
    struct sigaction action;
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
}

void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext){
    printf("\nreceived sigint, going to stop\n");
    exit(0);
}

void get_sem_and_shm(int *sem_id, int *shm_id){
    key_t key = ftok(FTOK_PATH, PROJ_ID);
    if((*sem_id = semget(key, semaphores_no, 0)) == -1)exit_error("Cant get semaphores");
    if((*shm_id = shmget(key, sizeof(pizzeria_t), 0)) == -1)exit_error("Cant get shared memory object");
}
