#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include <signal.h>
#include <fcntl.h> 
#include <sys/mman.h>
#include <semaphore.h>
#define oven_capacity 5
#define table_capacity 5

#define pizzerman_no 2
#define deliveryman_no 1

#define max_pizza 9
#define min_pizza 0

#define SHARED_MEMORY "/SHARED_M"

#define semaphores_no 5
#define sem_oven_not_in_use "/SEM_OVEN_NOT_IN_USE"
#define sem_table_not_in_use "/SEM_TABLE_NOT_IN_USE"
#define sem_table_free_space "/SEM_TABLE_FREE_SPACE"
#define sem_oven_free_space "/SEM_OVEN_FREE_SPACE"
#define sem_pizzas_on_table_count "SEM_PIZZAS_ON_TABLE_COUNT"


typedef struct{
    int oven_free_index;
    int table_free_index;
    int oven[oven_capacity];
    int table[table_capacity];
} pizzeria_t;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

typedef struct sembuf sembuf;

int rand_sleep(int min, int max);
int rand_pizza();
void exit_error(char *content);
char* get_time();
void set_sigint_handling(void (*func)(int,  siginfo_t *, void *));
void sigint_handler(int sig, siginfo_t *sig_inf, void *ucontext);
void get_sem_and_shm(int *shm_id, sem_t** table_free_space, sem_t** pizzas_on_table_count,sem_t** table_not_in_use, sem_t** oven_not_in_use,  sem_t** oven_free_space);
int get_sem_value(sem_t *restrict sem);
#endif 