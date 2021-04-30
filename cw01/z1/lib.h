#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>

struct Block{
    char **lin;
    int linNum;
};

struct BlockArr
{
    struct Block **blocks;
    int size;
    int capacity;
};


struct BlockArr* createBlockArr(int size);
struct Block* createBlock(int linNum);
double countTime(clock_t start, clock_t end);
int deleteBlockArr(struct BlockArr* arr);
int deleteBlock(struct BlockArr * arr, int index);
int deleteLine(struct BlockArr * arr, int indexBlock, int indexLin);
int mergeFilesSeq(int filesStart, int filesStop, char *argv[],  struct BlockArr *arr);
int mergeFiles(char *file1, char *file2);
int parseTmpToBlockArr(char *tmpFile, struct BlockArr *arr);
void printBlocks( struct BlockArr* arr);
#endif