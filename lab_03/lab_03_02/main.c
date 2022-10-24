#include "process.h"

int main(int argc, char *argv[])
{
    FILE *f = fopen("pos_01_in.txt", "r");
    int cnt = 0;
    int result = process(f, &cnt);
    if (argc != 2)
    if (f == NULL)
    {
        printf("Input Error");
        fclose(f);
        return result = INPUT_ERROR;
    }
    if (result == 0)
    {
        printf("%d", cnt);
        fclose(f);
        return EXIT_SUCCESS;
    }
    printf("Input Error");
    fclose(f);
    return result;
}
