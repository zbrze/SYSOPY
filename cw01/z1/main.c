#include <stdlib.h>
#include <stdio.h>
#include "lib.h"
int main(int argc, char *argv[]){
    if(argc < 2){
        printf("nie podano plikow");
        return;
            }
    struct BlockArr *arr = createBlockArr(2);
    mergeFiles(argv[1], argv[2]);
    //printBlocks(arr);
}