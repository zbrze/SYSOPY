#ifndef LAB4_LIBRARY_H
#define LAB4_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>

void handler(int sig);
void check_if_ignoring(int sig);
void ignore(int sig);
void handler_testing(int sig);
void check_if_waiting(int sig);
void mask(int sig);
void pending(int sig);
#endif