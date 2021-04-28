#include "header.h"
int sem_id;
int shm_id;
int pizza_type;
int curr_oven_index, curr_table_index;
sembuf free_oven = {sem_oven_not_in_use, 1, 0};
sembuf free_table = {sem_table_not_in_use, 1, 0};
sembuf put_in_oven[2] = {{sem_oven_not_in_use, -1, 0}, {sem_oven_free_space, -1, 0}};
sembuf take_from_oven[2] = {{sem_oven_not_in_use, -1, 0}, {sem_oven_free_space, 1, 0}};
sembuf put_on_table[2] = {{sem_table_not_in_use, -1, 0}, {sem_table_free_space, -1, 0}};
pizzeria_t *pizzas;
char* time_str;


void make_pizza(){
    pizza_type = rand_pizza();
    time_str = get_time();
    printf("*pizzerman %d* %s: przygotowuje pizze %d\n", getpid(), time_str, pizza_type);
    usleep(rand_sleep(1, 2));
    fflush(stdout);
}

void bake_pizza(){
    if(semop(sem_id, put_in_oven, 2) == -1) exit_error("Cannot put pizza in oven"); //will wait till it is possible to subtract
    pizzas = shmat(shm_id, NULL, 0);
    
    curr_oven_index = -1;
    for(int i = (pizzas->oven_free_index ); i < oven_capacity; i++){
        if(pizzas->oven[i] == -1){
            curr_oven_index = i;
            break;
        }
    }
    if(curr_oven_index == -1){
        for(int i = 0; i < pizzas->oven_free_index; i++){
            if(pizzas->oven[i] == -1){
                curr_oven_index = i;
                break;
            }
        }
    }
    
    pizzas->oven_free_index = (curr_oven_index + 1) % oven_capacity;
    pizzas->oven[curr_oven_index] = pizza_type;
    pizzas->pizzas_in_oven += 1;
    shmdt(pizzas);
    
    time_str = get_time();
    printf("*pizzerman %d* %s: dodalem pizze %d. Liczba pizz w piecu: %d\n", getpid(), time_str, pizza_type,  oven_capacity - semctl(sem_id, sem_oven_free_space, GETVAL));
    
    if(semop(sem_id, &free_oven, 1) == -1) exit_error("Unable to unlock oven");
    
    usleep(rand_sleep(4, 5));
}

void take_pizza_from_oven(){
    if(semop(sem_id, take_from_oven, 2) == -1) exit_error("Cannot put pizza in oven"); //will wait till it is possible to subtract

    pizzas = shmat(shm_id, NULL, 0);
    pizza_type = pizzas->oven[curr_oven_index];
    pizzas->oven[curr_oven_index] = -1;
    pizzas->pizzas_in_oven -= 1;
    shmdt(pizzas);
    printf("Wyjmuje pizze\n");
    if(semop(sem_id, &free_oven, 1) == -1) exit_error("Unable to unlock oven");
}

void put_away_pizza(){
    if(semop(sem_id, put_on_table, 2) == -1) exit_error("Cannot put pizza on table");

    pizzas = shmat(shm_id, NULL, 0);
    curr_table_index = (pizzas->table_free_index + 1) % table_capacity;
    pizzas->table[curr_table_index] = pizza_type;
    pizzas->pizzas_on_table += 1;
    pizzas->table_free_index = curr_table_index;
    printf("*pizzerman %d* %s: Wyjmuje pizze %d . Liczba pizz na stole: %d\n", getpid(), time_str, pizza_type, table_capacity - semctl(sem_id, sem_table_free_space, GETVAL));
    for(int i = 0; i < table_capacity; i++){
        printf("%d %d     ", i, pizzas->table[i] );
    }
    shmdt(pizzas);

    if(semop(sem_id, &free_table, 1) == -1) exit_error("Unable to unlock oven");

    time_str = get_time();
    
}

int main(){
    srand(getpid());
    get_sem_and_shm(&sem_id, &shm_id);
    while(1){
        make_pizza();
        bake_pizza();
        take_pizza_from_oven();
        put_away_pizza();
    }
}