#ifndef OTHER_H
#define OTHER_H

//#include "main.h"
#include <stdlib.h>			//rand
#include <sys/time.h>		//gettimeofday
#include <stdio.h>			//fprintf
#include <sys/resource.h>	//getrusage rusage
#include <stdint.h>			//uint64_t
#include <signal.h>			//SIGALRM

#define DIMENSIONS (const char*[]){"Б", "КБ", "МБ",  "ГБ", "ТБ", "ПБ"}
#define DIMENSIONS_COUNT 6

#define maxnum(x,y) ((x) > (y) ? (x) : (y))

int getrand(int min, int max);

void DieWithError(char *errorMessage);
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
void load_meter(int sig);
void human_readable (uint64_t size, char* str);

#endif
