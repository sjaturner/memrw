#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static volatile unsigned char array[0x10] = { 1, 2, 3, 4, 5, 6, 7, 8, -8, -7, -6, -5, -4, -3, -2, -1 };

void dump(volatile void *base, uint32_t len)
{
    volatile uint8_t *ptr = base;
    while (len--)
    {
        printf("%02x ", *ptr++);
    }
    printf("\n");
}

int main()
{
    printf("array is at %p\n", array);
    for (;;)
    {
        dump(array, sizeof array);
        sleep(1);
    }
    return 0;
}
