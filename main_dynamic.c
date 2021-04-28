#include <stdlib.h>
#include <stdio.h>
#include "../z1/lib.h"
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <dlfcn.h>

clock_t startTime, endTime;
struct tms startTms, endTms;

int isNum(char *arg){
    int flag = 1;
    int i = 0;
    while(i < strlen(arg) && flag){
        if(arg[i] < '0' || arg[i] > '9') flag = 0;
        i++;
    }
    return flag;
}

void timerStart(){
    startTime = times(&startTms);
}

void timerStop(){
    endTime = times(&endTms);
}


int main(int argc, char *argv[]){
    void *handle = dlopen("./lib.so", RTLD_LAZY);
    if (handle == NULL){
        printf("nie znaleziono lib.so");
        exit(EXIT_FAILURE);
    } 

    struct BlockArr* (*createBlockArr)() = dlsym(handle, "createBlockArr");
    int (*deleteBlockArr)() = dlsym(handle, "deleteBlockArr");
    int (*deleteLine)() = dlsym(handle, "deleteLine");
    int (*deleteBlock) () = dlsym(handle, "deleteBlock");
    int (*mergeFilesSeq)()  = dlsym(handle, "mergeFilesSeq");
    double (*countTime) () = dlsym(handle, "countTime");

    if(argc < 3){
        printf("podano za malo danych");
        return 0;
    }

    char *operation;
    struct BlockArr *arr = NULL;
    FILE *output = fopen("results3a.txt", "a");
    for(int i = 1; i < argc; i++){
        timerStart();
        char *command = argv[i];
        if(strcmp(command,"createBlockArr") == 0){
            if(arr != NULL){
                printf("Juz stworzono tablice blokow\n");
            }
            if(argc <= i + 1 || !isNum(argv[i+1])){
                printf("brak lub cledny rozmiar tablicy\n");
                break;
            }
            i++;
            arr = createBlockArr(atoi(argv[i]));
            printf("Tablica o rozmiarze %s zostala stworzona\n", argv[i]);
            operation = "create table";
        }

        else if(strcmp(command,"mergeFiles") == 0){
            if(argc <= i + 1){
                printf("nie podano plików");
                break;
            }
            i++;
            int filesCount = 0;
            int flag = 1;
            int j = i;
            while(j < argc && flag){
               if(strchr(argv[j], ':') == NULL){
                   flag = 0;
               }else{
                   filesCount++;
               }
               j++;
            }
            double *mergeTime;
            double *parseTime;
            if(mergeFilesSeq(i, filesCount + i, argv, arr, &mergeTime, &parseTime)){
                printf("Udalo sie polaczyc pliki i wpisac do blokow\n");
            }
            operation = "merge files";
            fprintf(output, "\nOperacja: %s\n", operation);
            fprintf(output, "Czas rzeczywisty: %f\n", mergeTime[0]);
            fprintf(output, "Czas uzytkownika: %f\n", mergeTime[1]);
            fprintf(output, "Czas systemowy: %f\n", mergeTime[2]);

            fprintf(output, "\nOperacja: parse tmp to blocks\n");
            fprintf(output, "Czas rzeczywisty: %f\n", mergeTime[0]);
            fprintf(output, "Czas uzytkownika: %f\n", mergeTime[1]);
            fprintf(output, "Czas systemowy: %f\n", mergeTime[2]);

            printf("\nOperacja: %s\n", operation);
            printf("Czas rzeczywisty: %f\n", mergeTime[0]);
            printf("Czas uzytkownika: %f\n", mergeTime[1]);
            printf("Czas systemowy: %f\n", mergeTime[2]);

            printf("\nOperacja: parse tmp to blocks\n");
            printf("Czas rzeczywisty: %f\n", mergeTime[0]);
            printf("Czas uzytkownika: %f\n", mergeTime[1]);
            printf("Czas systemowy: %f\n", mergeTime[2]);
            i += filesCount - 1;
        }
        
        else if(strcmp(command,"deleteBlock") == 0){
             if(argc <= i + 1 || !isNum(argv[i+1])){
                printf("brak lub bledny indeks bloku");
                break;
            }
            i++;
            if(deleteBlock(arr, atoi(argv[i]))){
                printf("poprawnie usunieto blok");
            }
            operation = "delete block";
        }
        else if(strcmp(command,"deleteLine") == 0){
             if(argc <= i + 2 || !isNum(argv[i+1]) || !isNum(argv[i+2])){
                printf("brak lub bledny indeks bloku/ wiersza");
                break;
            }
            i++;
            if(deleteLine(arr, atoi(argv[i]), atoi(argv[i+1]))){
                printf("poprawnie usunieto wiersz");
            }
            operation = "delete line";
            i++;
        }else{
            printf("podano niepoprawna operacje");
            break;
        }
        timerStop();
        if(strcmp(operation, "merge files")){
            fprintf(output, "\nOperacja: %s\n", operation);
            fprintf(output, "Czas rzeczywisty: %f\n",countTime(startTime, endTime));
            fprintf(output, "Czas uzytkownika: %f\n", countTime(startTms.tms_utime, endTms.tms_utime));
            fprintf(output, "Czas systemowy: %f\n", countTime(startTms.tms_stime, endTms.tms_stime));

            printf("\nOperacja: %s\n", operation);
            printf("Czas rzeczywisty: %f\n",countTime(startTime, endTime));
            printf("Czas uzytkownika: %f\n", countTime(startTms.tms_utime, endTms.tms_utime));
            printf("Czas systemowy: %f\n", countTime(startTms.tms_stime, endTms.tms_stime));
        }
    }
    deleteBlockArr(arr);
    dlclose(handle);
}