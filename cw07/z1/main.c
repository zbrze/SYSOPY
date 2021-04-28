#include "header.h"
#include<sys/stat.h>
#include <errno.h>
int sem_id;
int shm_id;
pid_t workers_pids[pizzerman_no + deliveryman_no];

void create_shm_and_sem(){
    key_t key = ftok(FTOK_PATH, 1);
    if((sem_id = semget(key, semaphores_no, IPC_CREAT | IPC_EXCL| 0666)) == -1){
        exit_error("Failed to create semaphores");
    }
    printf("KEY: %d\n", key);
    union semun sem_args;
    sem_args.val = 1;
    if(semctl(sem_id, sem_oven_not_in_use, SETVAL, sem_args) == -1) exit_error("Cant set oven in use semaphore");
    if(semctl(sem_id, sem_table_not_in_use, SETVAL, sem_args) == -1) exit_error("Cant set table in use semaphore");
    sem_args.val = oven_capacity;
    if(semctl(sem_id, sem_oven_free_space, SETVAL, sem_args) == -1) exit_error("Cant set oven occupied semaphore"); 
    sem_args.val = table_capacity;
    if(semctl(sem_id, sem_table_free_space, SETVAL, sem_args) == -1) exit_error("Cant set table occupied semaphore");

    //printf("%d", semctl(sem_id, sem_oven_in_use,GETVAL));
    if((shm_id = shmget(key, sizeof(pizzeria_t), IPC_CREAT | IPC_EXCL | 0666)) == -1){
        exit_error("Failed to create shm object");
    }
    pizzeria_t *pizzeria = shmat(shm_id, NULL, 0);
    pizzeria->pizzas_in_oven = 0;
    pizzeria->pizzas_on_table = 0;
    pizzeria->oven_free_index = 0;
    pizzeria->table_free_index = 0;
    for(int i = 0; i < table_capacity; i++) pizzeria->table[i] = -1;
    for(int i = 0; i < oven_capacity; i++) pizzeria->oven[i] = -1;
    shmdt(pizzeria);
}

void clean_shm_and_sem(){
    semctl(sem_id, 0, IPC_RMID, NULL);
    shmctl(shm_id, IPC_RMID, NULL);
}

int main(int argc, char **argv){
    if(atexit(clean_shm_and_sem) != 0) exit_error("Atexit setting failure");
    set_sigint_handling(sigint_handler);
    create_shm_and_sem();
    
    printf("Start creating pizzermen\n");
    for(int i = 0; i < pizzerman_no; i++){
        pid_t child = fork();
        if(child == 0){
            execl("./pizzerman", "./pizzerman", NULL);
            exit_error("Execl failure");
        }
        workers_pids[i] = child;

    }
    for(int i = 0 ; i < pizzerman_no; i++){
        wait(NULL);
    }
   
}