#include <stdio.h>
#include "file_find_max.h"
#include "file_find_min.h"
#include "file_how_many_bigger.h"
#define OK 0
#define ERR_IO -1

int main(int argc, char **argv)
{
    double max, min, avg;
    int result = 0;
    int check = (file_find_max("input.txt", &max) || file_find_min("input.txt", &min));
    if (check != OK)
        return check;
    avg = (max + min) / 2;
    file_how_many_bigger("input.txt", avg, &result);
    printf("%d\n", result);
    return 0;
}
