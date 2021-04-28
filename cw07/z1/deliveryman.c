#include "header.h"
int sem_id;
int shm_id;
int pizza_type;
int curr_oven_index, curr_table_index;
sembuf free_table = {sem_table_not_in_use, 1, 0};
sembuf take_from_table[3] = {{sem_table_not_in_use, -1, 0}, {sem_table_free_space, 1, 0}, {sem_pizzas_on_table_count, -1, 0}};
pizzeria_t *pizzas;
char* time_str;

void take_pizza_from_table(){
    if(semop(sem_id, take_from_table, 3) == -1) exit_error("Cannot take pizza from table"); //will wait till it is possible to subtract
    pizzas = shmat(shm_id, NULL, 0);
    pizza_type = pizzas->table[pizzas->table_free_index];
    pizzas->table[pizzas->table_free_index] = -1;
    shmdt(pizzas);
    time_str = get_time();
    printf("*deliveryboy %d* %s: pobieram pizze %d. Liczba pizz na stole: %d\n", getpid(), time_str, pizza_type,  semctl(sem_id, sem_pizzas_on_table_count, GETVAL));
    if(semop(sem_id, &free_table, 1) == -1) exit_error("Cannot unlock table"); //will wait till it is possible to subtract
    
    usleep(rand_sleep(4, 5));
}

void deliver_pizza_to_customer(){
    time_str = get_time();
    printf("*deliveryboy %d* %s: dostarczam pizze: %d.\n", getpid(), time_str, pizza_type);
    usleep(rand_sleep(4,5));
    
}
int main(){
    srand(getpid());
    get_sem_and_shm(&sem_id, &shm_id);
    while(1){
        take_pizza_from_table();
        deliver_pizza_to_customer();
    }
}