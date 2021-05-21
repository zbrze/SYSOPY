#include "header.h"
int** img_arr;
int **negative;
int width;
int height;
int max_color_val;
int thread_no = 0;
pthread_t* threads_ids;
int *arguments;
void *numbers_method(void *arg);
void *blocks_method(void *arg);
void* (*modes[2]) (void* arg) = {numbers_method, blocks_method};
void load_img(char* img_name){
    FILE *file = fopen(img_name, "r");
    if(file == NULL){
        exit_error("File doesn't exist");
    }
    char *buff = NULL;
    size_t buf_size = 0;
    for(int i = 0; i < 3; i++){
       getline(&buff, &buf_size, file);
       if(buff[0] == '#') i--;
       if(i == 1){
           char* tmp = strtok(buff, " ");
           width = atoi(tmp);
           tmp = strtok(NULL, " ");
           height = atoi(tmp);
       }
       if(i == 2){
           max_color_val = atoi(buff);
       }
    }
    img_arr = calloc(height, sizeof(int *) );
    for(int i = 0; i < height; i++){
        img_arr[i] = calloc(width, sizeof(int));
        for(int j = 0; j < width; j++){
            fscanf(file, "%d", &img_arr[i][j]);
        }
    }
    fclose(file);
    printf("%dx%d\n", width, height);
}

void *numbers_method(void *arg){
    struct timeval t_beginning, t_end, t_diff;
    gettimeofday(&t_beginning, NULL);
    int index = *((int *)arg);
    int step =  ceil((double) 255 / thread_no);
    int start = index * step + (index ? 1:0);
    int stop = (index + 1) * step > 255 ;
    if(stop > 255) stop = 255;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if(img_arr[i][j] >= start && img_arr[i][j] <= stop){
                negative[i][j] = 255 - img_arr[i][j];
            }  
        }
    }
    gettimeofday(&t_end, NULL);
    timersub(&t_end, &t_beginning, &t_diff);
    int *res = calloc(1, sizeof(int));
    *res = t_diff.tv_sec* 1000000. + t_diff.tv_usec;
    return res;
}

void *blocks_method(void *arg){
    struct timeval t_beginning, t_end, t_diff;
    gettimeofday(&t_beginning, NULL);
    

    int index = *((int *)arg);
    int start = index * ceil((double) width / thread_no) + (index?1:0);
    int stop = (index + 1) * ceil((double) width / thread_no);
    if(stop > width) stop = width;
    
    for(int i = 0; i < height; i++){
        for(int j = start; j <= stop; j++){
            negative[i][j] = 255 - img_arr[i][j];
        }
    }
    gettimeofday(&t_end, NULL);
    timersub(&t_end, &t_beginning, &t_diff);
    long *res = calloc(1, sizeof(long));
    *res = t_diff.tv_sec* 1000000. + t_diff.tv_usec;
    return res;
}


void cleanup(){
    for(int i = 0; i < height; i++){
        free(img_arr[i]);
    }
    free(img_arr);
    for(int i = 0; i < thread_no; i++){
        free(negative[i]);
    }
    free(negative);
    free(threads_ids);
    free(arguments);
}

void convert_time(long *sec, long *milisec, long *microsec){
    (*sec) = (*microsec) /1000000.0;
    (*microsec) -= (*sec) * 1000000.0;
    *milisec = (*microsec) / 1000.0;
    *microsec -= *milisec * 1000.0;  
}

int main(int argc, char** argv){
    if(argc != 5){
        printf("Wrong number of arguments");
        return -1;
    }
    modes[0] = numbers_method;
    thread_no = atoi(argv[1]);
    int mode_no = str_to_mode(argv[2]);
    if(mode_no == -1){
        printf("wrong mode");
        return -1;
    }
    char* input = argv[3];
    char* output = argv[4];
    load_img(input);
   
    negative = calloc(height, sizeof(int*));
    for (int i = 0; i < height; i++){
        negative[i] =  calloc(width, sizeof(int));
    }
   
    struct timeval t_start, t_end, t_diff;
    gettimeofday(&t_start, NULL);

    threads_ids = malloc(thread_no * sizeof(pthread_t));
    arguments = malloc(thread_no * sizeof(int));
    for(int i = 0; i < thread_no; i++){
        arguments[i] = i;
        if(pthread_create(&threads_ids[i], NULL, modes[mode_no], &arguments[i]) != 0){
            exit_error("Unable to create thread");
        }

    }
    
    FILE *times_file = fopen("times.txt", "a");
    if(times_file == NULL){
        exit_error("Cannot open output file");
    }
    fprintf(times_file, "---------------mode: %s --- threads: %d --- img size: %d x %d---------------------\n", argv[2], thread_no, width, height);
    long *microsec;
    long milisec, sec;
    for(int i = 0; i < thread_no; i++){
        if(pthread_join(threads_ids[i], (void **) &microsec) != 0){
            exit_error("Unable pthread_join");
        }
        convert_time(&sec, &milisec, microsec);
        fprintf(times_file, "Thread no: %ld, run time: %02ld:%03ld:%03ld\n", threads_ids[i], sec, milisec, *microsec);
        printf("Thread no: %ld, run time: %02ld:%03ld:%03ld\n", threads_ids[i], sec, milisec, *microsec);
    }

    gettimeofday(&t_end, NULL);
    timersub(&t_end, &t_start, &t_diff);
    *microsec = t_diff.tv_sec* 1000000. + t_diff.tv_usec;
    convert_time(&sec, &milisec, microsec);
    printf("FULL TIME: %02ld:%03ld:%03ld\n\n", sec, milisec, *microsec);
    fprintf(times_file, "FULL TIME: %02ld:%03ld:%03ld\n\n", sec, milisec, *microsec);
    fprintf(times_file, "\n");
    fclose(times_file);

    FILE* negative_file = fopen(output, "w");
    fprintf(negative_file, "P2\n%d %d\n255\n", width, height);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fprintf(negative_file, "%d ", negative[i][j]);
        }
    }
    fclose(negative_file);
    cleanup();
    printf("Exiting\n");
}
