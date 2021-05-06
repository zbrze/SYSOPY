#include "header.h"

int rand_sleep(int min, int max){
    min *= 1000000;
    max *= 1000000;
    return ((rand() % (max - min) + min));
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


void get_sem_and_shm(int *shm_id, sem_t** table_free_space, sem_t** pizzas_on_table_count,sem_t** table_not_in_use, sem_t** oven_not_in_use,  sem_t** oven_free_space){
    if((*shm_id = shm_open(SHARED_MEMORY, O_RDWR, 0666)) == -1) exit_error("Unable to get shm object");
    *table_free_space = sem_open(sem_table_free_space, O_RDWR, 0666);
    *pizzas_on_table_count = sem_open(sem_pizzas_on_table_count, O_RDWR, 0666);
    *table_not_in_use = sem_open(sem_table_not_in_use, O_RDWR, 0666);
    if(oven_not_in_use != NULL) *oven_not_in_use = sem_open(sem_oven_not_in_use, O_RDWR, 0666);
    if(oven_free_space != NULL) *oven_free_space = sem_open(sem_oven_free_space, O_RDWR, 0666);
}
int get_sem_value(sem_t *restrict sem){
    int val;
    sem_getvalue(sem , &val);
    return val;
}
