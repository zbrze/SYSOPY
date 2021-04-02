#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
clock_t startTime, endTime;
struct tms startTms, endTms;

double countTime(clock_t start, clock_t end){
    return ( (double) (end - start)/(sysconf(_SC_CLK_TCK)) );
}

struct BlockArr* createBlockArr(int size){
    if(!size){
        printf("Podano niewlasciwy rozmiar tablicy blokow\n");
        exit(-1);
    }
    struct BlockArr* arr = (struct BlockArr*) calloc(1, sizeof(struct BlockArr));
    arr -> size = 0;
    arr -> capacity = size;
    arr -> blocks = (struct Block**) calloc(size, sizeof(struct Block));
    return arr;
};

struct Block* createBlock( int linNum){
    struct Block* block = (struct Block*) calloc(1, sizeof(struct Block));
    block -> linNum = linNum;
    block -> lin = (char**) calloc(linNum, sizeof(char*));
    return block;
}



//wyrzucamy 0 kiedy lineracja się powiodla
int deleteBlockArr(struct BlockArr* arr){
    if(arr == NULL){
        printf("nie podano tablicy\n");
        return -1;
    }
    for(int i = 0; i < arr -> size; i++){
        deleteBlock(arr, i);
    }
    free(arr);
    return 0;
}

int deleteBlock(struct BlockArr * arr, int index){
    if(index > arr -> size || arr == NULL || index < 0 || arr -> blocks[index] == NULL){
        printf("nie mozna wykonać operacji usunięcia bloku\n");
        return 0;
    }
    free(arr -> blocks[index]);
    arr -> blocks[index] = NULL;
    return 1;
}

int deleteLine(struct BlockArr * arr, int indexBlock, int indexLin){
     if(indexBlock > arr -> size || arr == NULL || indexBlock*indexLin  < 0 || arr -> blocks[indexBlock] == NULL ){
        printf("nie mozna wykonać operacji usunięcia linii\n");
        return 0;
    }
    free(arr -> blocks[indexBlock] -> lin[indexLin]);
    arr -> blocks[indexBlock] -> lin[indexLin] = NULL;
    return 1;
}

/*Przeprowadzenie zmergowania (dla kazdego elementu sekwencji) oraz zapisanie wyniku zmergowania do pliku tymczasowego*/
int mergeFiles(char *file1, char *file2, struct BlockArr * arr){
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");
    
    FILE *tmp = NULL;
    tmp = fopen("tmp.txt", "w");
    if(f1 == NULL || f2 == NULL){
        printf("nie mozna otworzyc pliku\n");
        return -1;
    }
    fseek(f1, 0L, SEEK_END);
    double size1 = ftell(f1);
    fseek(f1, 0L, SEEK_SET);
    fseek(f2, 0L, SEEK_END);
    double size2 = ftell(f2);
    fseek(f2, 0L, SEEK_SET);
    char *buff1 = (char* ) calloc(2*size1, sizeof(char));
    char *buff2 = (char* ) calloc(2* size2, sizeof(char));
    int flag1 = 1;
    int flag2 = 1;
    char *operation = (char* ) calloc(2* (size2 + size1), sizeof(char));
    
    while (flag1 || flag2)
    {
        if(fgets(buff2, 400, f2)){
            strcat(operation,buff2);
        }else{
            flag1 = 0;
        }
        if(fgets(buff1, 400, f1)){
            strcat(operation, buff1);
        }else{
            flag2 = 0;
        }
        
    }
    free(buff1);
    free(buff2);

    fputs(operation, tmp);
    fclose(f1);
    fclose(f2);
    fclose(tmp);
    free(operation);
    return 1;
    
}

void print_pid(char* file_name, int pid){
    FILE *f = fopen(file_name, "a");
    fprintf(f, "%d,  ", pid);
    fclose(f);
}

int mergeFilesSeq(int filesStart, int filesStop, char *argv[], struct BlockArr * arr){
    
    for(long i = filesStart; i < filesStop; i++){
        char *f1 = strtok(argv[i], ":");
        char *f2 = strtok(NULL, " ");
        pid_t child;
        if((child = fork()) == 0){
           mergeFiles(f1, f2, arr);
           printf("\n\nmerge: %d %s %s", getpid(), f1, f2);
          print_pid("raport.txt", getpid());
           exit(0);
        }
        
        parseTmpToBlockArr("tmp.txt", arr);
        wait(NULL);
        
    }
    return 1;
}

/*Utworzenie, na podstawie zawartość pliku tymczasowego, bloku wierszy — tablicy wskaźnikow na wiersze, ustawienie w tablicy glownej 
(wskaźnikow) wskazania na ten blok; na końcu, funkcja powinna zwrocić indeks elementu tablicy (glownej), ktory zawiera wskazanie na utworzony blok*/
int parseTmpToBlockArr(char *tmpFile, struct BlockArr *arr){
    FILE *f = fopen(tmpFile, "r");
    if(f == NULL){
        printf("nie istnieje plik\n");
        return -1;
    }
    if(arr == NULL){
        printf("nie utworzono tablicy\n");
        return -1;
    }
    double size = 0;
    double count = 0;
    fseek(f, 0L, SEEK_END);
    int size1 = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *buff = (char* ) calloc(size1 + 1, sizeof(char));
    char *operation = (char* ) calloc(size1 + 1, sizeof(char));  
    //char operation[10000] = ""1;
    //char buff[400];
    while(fgets(buff, size1, f)){
        strcat(operation, buff);
        count++;
        size += strlen(buff);
    }
    fclose(f);
    struct Block* b = createBlock(count);
    long iter = 0;
    long op_iter = 0;
    long len = 0;
    for(iter = 0; iter < count; iter++)
    {
        len = op_iter;
        while(operation[op_iter] != '\n')
        {
            op_iter++;
        }
        len = op_iter - len;

        b->lin[iter] = (char*)calloc(len, sizeof(char));
        
        
        int cpy_iter = 0;
        for(cpy_iter = 0; cpy_iter < len; cpy_iter++){
           b->lin[iter][cpy_iter] = operation[op_iter-len+cpy_iter];
        }

        //nowa linia
        op_iter++;    
    }
    b->linNum = count;
    arr -> blocks[arr -> size ] = b;
    arr -> size +=1;
    free(operation);
    free(buff);
    return 1;
}

void printBlocks( struct BlockArr* arr){
    if(arr == NULL){
        printf("niepoprawne dane\n");
        return;
    }
    for(int i = 0; i < arr -> size; i++){
        printf("\n%d blok\n", i);
      if(arr -> blocks[i] != NULL){
           for(int j = 0; j < arr -> blocks[i] -> linNum; j++){
               if( arr ->blocks[i] -> lin[j] != NULL){
                   printf("%s\n", arr ->blocks[i] -> lin[j]);
               }
           }
      }
     
    }


}