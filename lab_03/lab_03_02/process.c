#include "process.h"

int process(FILE *f, int *cnt)
{
    double max_i = -1000000000, min_i = 1000000000;
    max_min_file(f, &max_i, &min_i);
    if (max_i == -1000000000 && min_i == 1000000000)
        return INPUT_ERROR;
    fseek(f, 0L, SEEK_SET);
    double average = (max_i + min_i) / 2;
    *cnt = 0;
    count(f, cnt, average);
    return 0;
}

void count(FILE *f, int *cnt, double average)
{
    double element;
    while (fscanf(f, "%lf", &element) == 1)
        *cnt += (element > average);
}

void max_min_file(FILE *f, double *max_i, double *min_i)
{
    double element;
    while (fscanf(f, "%lf", &element) == 1)
    {
        if (element > *max_i)
            *max_i = element;
        if (element < *min_i)
            *min_i = element;
    }
}