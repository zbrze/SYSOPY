#include "header.h"
int sem_id;
int shm_id;
int pizza_type;
int curr_oven_index, curr_table_index;
sem_t* table_not_in_use;
sem_t* table_free_space;
sem_t* pizzas_on_table_count;
pizzeria_t *pizzas;
char* time_str;

void take_pizza_from_table(){
    printf("PIZZA: table: %d %d %d\n", get_sem_value(table_not_in_use), get_sem_value(pizzas_on_table_count), get_sem_value(table_free_space));
    fflush(stdout);
    if(sem_wait(pizzas_on_table_count)==-1) exit_error("Cannot put pizza in oven");
    if(sem_wait(table_not_in_use)==-1) exit_error("Cannot put pizza in oven"); //will wait till it is possible to subtract
    if(sem_post(table_free_space)==-1) exit_error("Cannot put pizza in oven");
    if((pizzas = mmap(NULL, sizeof(pizzeria_t), PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED) exit_error("Cannot get shm object");
    
    pizza_type = pizzas->table[pizzas->table_free_index];
    pizzas->table[pizzas->table_free_index] = -1;
    munmap(pizzas, sizeof(pizzeria_t));
    time_str = get_time();
    printf("*deliveryboy %d* %s: pobieram pizze %d. Liczba pizz na stole: %d\n", getpid(), time_str, pizza_type, get_sem_value(pizzas_on_table_count));
    if(sem_post(table_not_in_use)==-1) exit_error("Unable to unlock oven");

    usleep(rand_sleep(4, 5));
}

void deliver_pizza_to_customer(){
    time_str = get_time();
    printf("*deliveryboy %d* %s: dostarczam pizze: %d.\n", getpid(), time_str, pizza_type);
    usleep(rand_sleep(4,5));
    
}
int main(){
    srand(getpid());
    get_sem_and_shm(&shm_id, &table_free_space, &pizzas_on_table_count, &table_not_in_use, NULL, NULL);
    while(1){
        take_pizza_from_table();
        deliver_pizza_to_customer();
    }
}