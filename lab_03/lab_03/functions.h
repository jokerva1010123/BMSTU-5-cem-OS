#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE 16
#define TITLE_SIZE 27

#define FILE_NAME_SIZE 100
#define FILTER_SIZE 100

#define LOWER_LIMIT_ARGC 2
#define UPPER_LIMIT_ARGC 3

#define OK 0
#define FILE_READING_ERROR 1
#define ARRAY_SIZE_ERROR 2
#define ARGS_ERROR 3
#define TITLE_SIZE_ERROR 4
#define WEIGHT_VOLUME_ERROR 5
#define FILE_CLOSE_ERROR 6

typedef struct
{
    char title[TITLE_SIZE];
    float weight;
    float volume;
} description;

int parse_args(int argc, char **argv, char *filename, char *filter);
int input_inform(description *items_array, int *items_len, FILE *file);
void print_structure(const description *const items_array, const int index);
void print_inform(const description *const items_array, const int items_len, int argc, char *filter);
void insertion_sort(description *items_array, int items_len);

#endif 

