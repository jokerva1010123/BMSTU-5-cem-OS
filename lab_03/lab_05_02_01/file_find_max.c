#include <stdio.h>
#define OK 0
#define ERR_IO -1
#define ERR_NO_DATA -2

int file_find_max(const char *filename, double *max)
{
    FILE *file = fopen(filename, "r");
    double number = 0;
    int check = 1, is_number = 0;
    char ch;
    if (file == NULL)
        return ERR_IO;
    while (!feof(file))
    {
        check = fscanf(file, "%lf", &number);
        if (check != 1)
            fscanf(file, "%c", &ch);
        else
        {
            if (!is_number || (is_number && number > *max))
                *max = number;
            is_number = 1;
        }
    }
    fclose(file);
    if (!is_number)
        return ERR_NO_DATA;
    return OK;
}