#include <stdio.h>
#include <iostream>
#define ERROR 1
int input(int *a, int n)
{
    printf("SORT: enter elements of array: ");
    int i;
    for (i = 0; i < n; i++)
        if (scanf("%d", &a[i]) != 1)
            return i;
    return i;
}
void sort(int *data, int length, int (*cmp)(int, int))
{
    bool fl;
    for (size_t j = 1; j < length; j++) {
        fl = false;
        for (size_t i = 0; i < length - j; i++) {
            if (cmp(data[i], data[i + 1]) > 0) {
                std::swap(data[i], data[i + 1]);
                fl = true;
            }
        }
        if (!fl)
            break;
    }
    return;
}
int compare_int(int f, int s)
{
    return f - s;
}
int main()
{
    int n;
    printf("SORT: enter the number of elements in array: ");
    scanf("%d", &n);
    int *a = (int *)malloc(n * sizeof(int));
    if (a == NULL)
        return ERROR;

    input(a, n);

    sort(a, n, compare_int);

    printf("sorted array: ");
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n");

    free(a);

    return 0;
}