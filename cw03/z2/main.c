#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<string.h>
#include "lib.h"
#include <time.h>
#include <sys/times.h>

clock_t startTime, endTime;
struct tms startTms, endTms;

void timerStart(){
    startTime = times(&startTms);
}

void timerStop(){
    endTime = times(&endTms);
}

int main(int argc, char** argv) {
    if(argc < 3){
        printf("podano za malo danych");
        return 0;
    }
    struct BlockArr *arr = NULL;
    arr = createBlockArr(atoi(argv[1]));
    FILE *output = fopen("raport.txt", "a");

    int filesCount = 0;
    int flag = 1;
    int j = 2;

    while(j < argc && flag){
        if(strchr(argv[j], ':') == NULL){
            flag = 0;
            
        }else{
            filesCount++;
        }
        j++;
    }
    timerStart();
    mergeFilesSeq(2, filesCount + 2, argv, arr);
    //printBlocks(arr);
    timerStop();
    deleteBlockArr(arr);
    fprintf(output, "\nCzas rzeczywisty: %f\n",countTime(startTime, endTime));
    fprintf(output, "Czas uzytkownika: %f\n", countTime(startTms.tms_utime, endTms.tms_utime));
    fprintf(output, "Czas systemowy: %f\n\n\n", countTime(startTms.tms_stime, endTms.tms_stime));
    fclose(output);
}
