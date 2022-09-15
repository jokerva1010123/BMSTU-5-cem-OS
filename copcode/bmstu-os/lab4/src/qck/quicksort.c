#include <stdio.h>

void quickSort(int *array, int size) 
{
    int i = 0;
    int j = size - 1;
    int mid = array[size / 2];

    while (i <= j)
    {
        while(array[i] < mid) 
            i++;

        while(array[j] > mid) 
            j--;

        if (i <= j) 
        {
            int tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;
            i++;
            j--;
        }
    }

    if (j > 0) 
    {
        quickSort(array, j + 1);
    }
    if (i < size) 
    {
        quickSort(&array[i], size - i);
    }
}

int main(void)
{
    int array[10] = {1, 5, 23, -4, 6, 3, 0, -5, 9, 11};
    printf("Исходный массив: \n");
    for (size_t i = 0; i < 10; i++)
        printf("%d ", array[i]);
    
    quickSort(array, 10);

    printf("\n\nПосле быстрой сортировки: \n");
    for (size_t i = 0; i < 10; i++)
        printf("%d ", array[i]);
    printf("\n");
    return 0;
}