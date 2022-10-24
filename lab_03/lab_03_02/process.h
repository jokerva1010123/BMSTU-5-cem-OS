#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <stdio.h>
#include <stdlib.h>

#define INPUT_ERROR -1


void max_min_file(FILE *f, double *max_i, double *min_i);
int process(FILE *f, int *cnt);
void count(FILE *f, int *cnt, double average);

#endif // !_PROCESS_H_
