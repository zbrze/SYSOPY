#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#define NUMBERS 0
#define BLOCK 1
#define INTERLEAVED 2

#define MAX_IMG_INFO_LINE 20

int str_to_mode(char* str);
char* mode_to_str(int mode);
void exit_error(char *content);

#endif 
