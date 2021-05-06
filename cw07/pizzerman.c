#include "header.h"
int shm_id;
int pizza_type;
int curr_oven_index, curr_table_index;
sem_t* oven_not_in_use;
sem_t* table_not_in_use;
sem_t* oven_free_space;
sem_t* table_free_space;
sem_t* pizzas_on_table_count;

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
    if(sem_wait(oven_not_in_use)==-1) exit_error("Cannot put pizza in oven"); //will wait till it is possible to subtract
    if(sem_wait(oven_free_space)==-1) exit_error("Cannot put pizza in oven");
    if((pizzas = mmap(NULL, sizeof(pizzeria_t), PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED) exit_error("Cannot get shm object");
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
    munmap(pizzas, sizeof(pizzeria_t));
    
    time_str = get_time();
    printf("*pizzerman %d* %s: dodalem pizze %d. Liczba pizz w piecu: %d\n", getpid(), time_str, pizza_type,  oven_capacity - get_sem_value(oven_free_space));
    
    if(sem_post(oven_not_in_use)==-1) exit_error("Unable to unlock oven");
    
    
    usleep(rand_sleep(4, 5));
}

void take_pizza_from_oven(){
    
    if(sem_wait(oven_not_in_use)==-1) exit_error("Cannot take pizza from oven");
    if(sem_post(oven_free_space)==-1) exit_error("Cannot take pizza from oven");

    if((pizzas = mmap(NULL, sizeof(pizzeria_t), PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED) exit_error("Cannot get shm object");
    pizza_type = pizzas->oven[curr_oven_index];
    pizzas->oven[curr_oven_index] = -1;
    munmap(pizzas, sizeof(pizzeria_t));
    if(sem_post(oven_not_in_use)==-1) exit_error("Cannot take pizza from oven");
}

void put_away_pizza(){

    if(sem_wait(table_free_space)==-1) exit_error("Cannot take pizza from oven");
    if(sem_wait(table_not_in_use)==-1) exit_error("Cannot take pizza from oven");
    if(sem_wait(oven_not_in_use)==-1) exit_error("Cannot take pizza from oven");
    if(sem_post(pizzas_on_table_count)==-1) exit_error("Cannot take pizza from oven");

    if((pizzas = mmap(NULL, sizeof(pizzeria_t), PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED) exit_error("Cannot get shm object");
    curr_table_index = (pizzas->table_free_index + 1) % table_capacity;
    pizzas->table[curr_table_index] = pizza_type;
    pizzas->table_free_index = curr_table_index;
    time_str = get_time();
    printf("*pizzerman %d* %s: Wyjmuje pizze %d . Liczba pizz na stole: %d\n", getpid(), time_str, pizza_type, get_sem_value(pizzas_on_table_count));

    munmap(pizzas, sizeof(pizzeria_t));
    if(sem_post(table_not_in_use)==-1) exit_error("Cannot take pizza from oven");
    if(sem_post(oven_not_in_use)==-1) exit_error("Cannot take pizza from oven");
}

void clenup(){
   sem_close(table_free_space);
   sem_close(pizzas_on_table_count);
   sem_close(oven_free_space);
   sem_close(table_not_in_use);
   sem_close(oven_not_in_use);
}

int main(){

    srand(getpid());
    get_sem_and_shm(&shm_id, &table_free_space, &pizzas_on_table_count, &table_not_in_use, &oven_not_in_use, &oven_free_space);
    while(1){
        make_pizza();
        bake_pizza();
        take_pizza_from_oven();
        put_away_pizza();
    }
}