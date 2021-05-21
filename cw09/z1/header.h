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
#include<sys/wait.h>
#include <unistd.h>

#define ELVES_NO 6
#define REINDEERS_NO 3
#define DELIVERY_NO 3

int rand_sleep(int min, int max);
void exit_error(char *content);

#endif 