#include"header.h"

int elves_waiting = 0;
int reindeers_back = 0;
int is_santa_asleep = 0;
int deliveries_done = 0;
int elves_waiting_arr[3] = {0, 0, 0};
int reindeers_asleep = 0;
pthread_t *reindeers_thread;
pthread_t *elves_thread;
pthread_t santa_thread;
pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_mutex_wait = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t elves_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elves_mutex_wait = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t wake_up_santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeers_delivering = PTHREAD_COND_INITIALIZER;
pthread_cond_t elves_resolving =PTHREAD_COND_INITIALIZER;

void* santa_func(void* arg){
  while(1){  
    pthread_mutex_lock(&santa_mutex);
    pthread_cond_wait(&wake_up_santa_cond, &santa_mutex);
    printf("Mikolaj: budze sie\n");
    is_santa_asleep = 0;
    pthread_mutex_lock(&reindeer_mutex);
    if(reindeers_back == REINDEERS_NO){
        printf("Mikolaj: Dostarczam zabawki\n");
        
        usleep(rand_sleep(2, 4));
        deliveries_done++;
        if(deliveries_done == 3){ 
          printf("Mikolaj: rozwiozlem prezenty 3 razy, koncze prace\n");
          exit(0);
          }
        reindeers_asleep = 0;
        reindeers_back = 0;
        if(pthread_cond_broadcast(&reindeers_delivering) != 0) exit_error("Broadcast err");
        pthread_mutex_unlock(&reindeer_mutex);
    }
        pthread_mutex_lock(&elves_mutex);
        if(elves_waiting == 3){

        printf("Mikolaj: pomagam elfom %d, %d, %d\n", elves_waiting_arr[0], elves_waiting_arr[1], elves_waiting_arr[2]);
        elves_waiting_arr[0] = elves_waiting_arr[1] = elves_waiting_arr[2] = 0;

        usleep(rand_sleep(1, 2));
        elves_waiting = 0;

        pthread_mutex_lock(&elves_mutex_wait);
        pthread_cond_broadcast(&elves_resolving);
        pthread_mutex_unlock(&elves_mutex_wait);

    }
      pthread_mutex_unlock(&reindeer_mutex);
    printf("Mikolaj: zasypiam\n");
    pthread_mutex_unlock(&santa_mutex);

    pthread_mutex_unlock(&elves_mutex);
  }
  pthread_exit((void*) 0);
}

void* reindeer_func(void *arg){
  int id =  *((int *)arg);
  while(1){
    pthread_mutex_lock(&reindeer_mutex_wait);
    while(reindeers_asleep) pthread_cond_wait(&reindeers_delivering, &reindeer_mutex_wait);
    pthread_mutex_unlock(&reindeer_mutex_wait);

    usleep(rand_sleep(3, 4)); //holiday
    pthread_mutex_lock(&reindeer_mutex);
    reindeers_back++;
    printf("Renifer %d: na mikolaja razem ze mna czeka %d reniferow\n", id, reindeers_back);
    reindeers_asleep = 1;

    if(reindeers_back == REINDEERS_NO){
      printf("Renifer %d: budze mikolaja\n", id);
      pthread_cond_broadcast(&wake_up_santa_cond);
    }
    pthread_mutex_unlock(&reindeer_mutex);
  }
}

void* elf_func(void* arg){
  int id =  *((int *)arg);
  while(1){
    usleep(rand_sleep(1, 2));
    pthread_mutex_lock(&elves_mutex_wait);
    while(elves_waiting >= 3 ){
      printf("Elf %d czeka na powrót elfów\n", id);
      pthread_cond_wait(&elves_resolving, &elves_mutex_wait);
    }
    pthread_mutex_unlock(&elves_mutex_wait);
    pthread_mutex_lock(&elves_mutex);
    if(elves_waiting < 3){
        elves_waiting_arr[elves_waiting] = id;
        elves_waiting++;
        printf("Elf %d: na mikolaja razem ze mna czeka %d elfow\n", id, elves_waiting);

    pthread_mutex_unlock(&elves_mutex);
        if(elves_waiting == 3){
          printf("Elf %d: budze mikolaja\n", id);
          pthread_cond_broadcast(&wake_up_santa_cond);
        }

    pthread_mutex_lock(&elves_mutex_wait);
        while(elves_waiting != 0 ){
          pthread_cond_wait(&elves_resolving, &elves_mutex_wait);
        }

    pthread_mutex_unlock(&elves_mutex_wait);
    }
  }
}
void clenup(){
  
}
int main(){
  reindeers_thread = calloc(REINDEERS_NO, sizeof(pthread_t));
  int* reindeers_ids= calloc(REINDEERS_NO, sizeof(int));
  int* elves_ids = calloc(ELVES_NO, sizeof(int));
  elves_thread = calloc(ELVES_NO, sizeof(pthread_t));

  pthread_create(&santa_thread, NULL, &santa_func, NULL);
  printf("Stworzono mikolaja\n");
  for(int i = 0; i < REINDEERS_NO; i++){
    reindeers_ids[i] = i + 10;
    pthread_create(&reindeers_thread[i], NULL, &reindeer_func, &reindeers_ids[i]);
    printf("Stworzono renifera %d\n", reindeers_ids[i]);
  }
  for(int i = 0; i < ELVES_NO; i++){
    elves_ids[i] = i + 20;
    pthread_create(&elves_thread[i], NULL, &elf_func, &elves_ids[i]);
    printf("Stworzono elfa %d\n", elves_ids[i]);
  }
  pthread_join(santa_thread, NULL);

 for (int i = 0; i < REINDEERS_NO; i++){
        pthread_join(reindeers_thread[i], NULL);
        sleep(1);
    }
 for (int i = 0; i < ELVES_NO; i++){
        pthread_join(elves_thread[i], NULL);
    }

  free(reindeers_thread);
  free(elves_thread);
}