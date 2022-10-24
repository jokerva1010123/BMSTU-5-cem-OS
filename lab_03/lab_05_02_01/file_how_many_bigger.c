#include <stdio.h>
#define OK 0
#define ERR_IO -1

int file_how_many_bigger(const char *filename, double max, int *result)
{
    FILE *file = fopen(filename, "r");
    double number = 0;
    int check = 1;
    char ch;
    *result = 0;
    if (file == NULL)
        return ERR_IO;
    while (!feof(file))
    {
        check = fscanf(file, "%lf", &number);
        if (check != 1)
            fscanf(file, "%c", &ch);
        else
        {
            if (number > max)
                (*result)++;
        }
    }
    fclose(file);
    return OK;
}