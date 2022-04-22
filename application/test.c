#include "linklayer.h"

#define INPUT_SIZE 8

int main()
{

    char input[INPUT_SIZE] = {0x12, 0x2, 0x3, 0x4, 0x7e, 0x13, 0x22, 0x1};
    // PRINT_V(input, INPUT_SIZE);

    size_t stuffed_size;
    char *stuffed = stuffing(input, INPUT_SIZE, &stuffed_size);

    PRINT_V(stuffed, stuffed_size);
    // printf("stuffed size %lu \n\n", stuffed_size);

    size_t destuffed_size;
    char *destuffed = destuffing(stuffed, stuffed_size, &destuffed_size);
    PRINT_V(destuffed, destuffed_size);
    printf("destuffed size %lu \n\n", destuffed_size);

    free(stuffed);
    free(destuffed);
}