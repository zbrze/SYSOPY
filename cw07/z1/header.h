#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include<sys/wait.h>
#include <signal.h>

#define oven_capacity 5
#define table_capacity 5

#define pizzerman_no 7
#define deliveryman_no 1

#define max_pizza 9
#define min_pizza 0

#define FTOK_PATH getenv("HOME")
#define PROJ_ID 1

#define semaphores_no 4
#define sem_oven_not_in_use 0
#define sem_table_not_in_use 1
#define sem_table_free_space 2
#define sem_oven_free_space 3

typedef struct{
    int oven_free_index;
    int table_free_index;
    int pizzas_in_oven;
    int pizzas_on_table;
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
void get_sem_and_shm(int *sem_id, int *shm_id);
#endif 