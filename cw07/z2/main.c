#include "header.h"
pid_t workers_pids[pizzerman_no + deliveryman_no];
const char* sem_names[semaphores_no] = {sem_oven_not_in_use, sem_table_not_in_use, sem_oven_free_space, sem_table_free_space, sem_pizzas_on_table_count};
const int sem_vals[semaphores_no] = {1, 1, oven_capacity, table_capacity, 0};
void create_shm_and_sem(){
    sem_t* sem;
    for(int i = 0; i < semaphores_no; i++){
        if((sem = sem_open(sem_names[i], O_CREAT | O_RDWR, 0666, sem_vals[i])) == SEM_FAILED) exit_error("Unable to create semaphore");
        sem_close(sem);
    }
    int shm;
    //printf("%d", semctl(sem_id, sem_oven_in_use,GETVAL));
    if((shm = shm_open(SHARED_MEMORY, O_CREAT | O_RDWR, 0666)) == -1){
        exit_error("Failed to create shm object");
    }
    if(ftruncate(shm, sizeof(pizzeria_t)) == -1) exit_error("Unable to set memory size of shm object");
    pizzeria_t *pizzeria = mmap(NULL, sizeof(pizzeria_t), PROT_WRITE, MAP_SHARED, shm, 0);
    pizzeria->oven_free_index = 0;
    pizzeria->table_free_index = 0;
    for(int i = 0; i < table_capacity; i++) pizzeria->table[i] = -1;
    for(int i = 0; i < oven_capacity; i++) pizzeria->oven[i] = -1;
    munmap(pizzeria, sizeof(pizzeria_t));
}

void clean_shm_and_sem(){
    for(int i = 0; i < semaphores_no; i++){
        if(sem_unlink(sem_names[i]) == -1) exit_error("Unable to delete semaphore");
    }
    if(shm_unlink(SHARED_MEMORY) == -1) exit_error("Unable to unlink shm object");
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

    printf("Start creating deliverymen\n");
    for(int i = 0; i < deliveryman_no; i++){
        pid_t child = fork();
        if(child == 0){
            execl("./deliveryman", "./deliveryman", NULL);
            exit_error("Execl failure");
        }
        workers_pids[i] = child;
    }
    for(int i = 0 ; i < pizzerman_no + deliveryman_no; i++){
        wait(NULL);
    }
   
}