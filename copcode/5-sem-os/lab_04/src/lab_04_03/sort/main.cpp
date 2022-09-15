#include <iostream>
#define ERROR 1
void swap(char *p1, char *p2, size_t size)
{
    if (!size || !p1 || !p1)
        return;
    char q;
    for (size_t i = 0; i < size; i++)
    {
        q = *p1;
        *p1 = *p2;
        *p2 = q;
        p1++;
        p2++;
    }
}
void sort(void *beg, size_t number, size_t size, int (*comparator)(const void *, const void *))
{
    if (beg == NULL || comparator == NULL)
        return;
    char *pb = (char *)beg;
    char *pe = pb + size * number;
    if (pb == pe)
        return;
    pe -= size;
    char *ptr = NULL, *max_ptr = NULL;
    while (pe > pb)
    {
        max_ptr = ptr = pe;
        while (ptr >= pb)
        {
            if (comparator(ptr, max_ptr) > 0)
                max_ptr = ptr;
            ptr -= size;
        }
        if (max_ptr != pe)
            swap(max_ptr, pe, size);
        pe -= size;
    }
    return;
}
int compare_int(const void *f, const void *s)
{
    int *a = (int *)(f);
    int *b = (int *)(s);
    return *a - *b;
}
int main(int argc, const char *argv[])
{
    if (argc - 1 <= 0)
        return ERROR;
    int *a = (int *)malloc((argc - 1) * sizeof(int));
    if (a == NULL)
        return ERROR;
    for (int i = 1; i < argc; i++)
        a[i - 1] = atoi(argv[i]);

    sort(a, argc - 1, sizeof (int), compare_int);

    printf("sorted array: ");
    for (int i = 0; i < argc - 1; i++)
        printf("%d ", a[i]);
    printf("\n");

    free(a);

    return 0;
}