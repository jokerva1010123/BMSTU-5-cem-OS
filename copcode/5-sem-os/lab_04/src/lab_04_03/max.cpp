#include <stdio.h>
#include <cstdlib>
#define ERROR 1
int input(int *a, int n)
{
    printf("MAX: enter elements of array: ");
    int i;
    for (i = 0; i < n; i++)
        if (scanf("%d", &a[i]) != 1)
            return i;
    return i;
}
int main()
{
    int n;
    printf("MAX: enter the number of elements in array: ");
    scanf("%d", &n);
    int *a = (int *)malloc(n * sizeof(int));
    if (a == NULL)
        return ERROR;
    input(a, n);
    int max = a[0], tmp;
    for (int i = 1; i < n; i++)
        if ((tmp = a[i]) > max)
            max = tmp;
    printf("max in array: %d\n", max);
    return 0;
}
