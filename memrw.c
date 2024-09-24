#define _GNU_SOURCE
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <errno.h>

void dump(uint8_t *ptr, uint32_t len)
{
    while (len--)
    {
        printf("%02x ", *ptr++);
    }
    printf("\n");
}

char *program_name;

void usage(int exit_code)
{
    printf("read usage:\n");
    printf("    %s pid address r count\n", program_name);
    printf("where:\n");
    printf("    pid is the PID of the process to read memory from\n");
    printf("    address is the virtual address within the process where the read starts, hexadecimal addresses require an 0x prefix\n");
    printf("    count is the number of bytes to read, these will be will be emitted as octets in hexadecimal on a single line\n");
    printf("write usage:\n");
    printf("    %s pid address w first_octet second_octet ...\n", program_name);
    printf("where:\n");
    printf("    pid is the PID of the process to write into\n");
    printf("    address is the virtual address within the process where the write will start, hexadecimal addresses require an 0x prefix\n");
    printf("    first_octet, etc. octets to be written to the start address and subsequent addresses the octets comprise two hexadecimal nibbles followed by space with no 0x prefix\n");
    exit(exit_code);
}

int main(int argc, char *argv[])
{
    struct iovec local[1] = { };
    struct iovec other[1] = { };

    program_name = (--argc, *argv++);

    if (argc <= 0)
    {
        usage(EXIT_SUCCESS);
    }

    int pid = strtol((--argc, *argv++), 0, 0);

    if (argc <= 0)
    {
        printf("Address missing\n");
        usage(EXIT_FAILURE);
    }

    intptr_t address = strtol((--argc, *argv++), 0, 0);

    if (argc <= 0)
    {
        printf("Specify 'r' or 'w' for read or write\n");
        usage(EXIT_FAILURE);
    }

    char read_write = *(--argc, *argv++);

    if (read_write == 'r')
    {
        local->iov_base = calloc(other->iov_len = local->iov_len = strtoull(*argv, 0, 0), 1);
        other->iov_base = (void *)address;

        if (process_vm_readv(pid, local, 1, other, 1, 0) < 0)
        {
            perror("process_vm_readv");
            exit(EXIT_FAILURE);
        }

        dump(local->iov_base, local->iov_len);
    }
    else if (read_write == 'w')
    {
        uint8_t *scan = local->iov_base = calloc(argc, 1);
        other->iov_len = local->iov_len = argc;
        other->iov_base = (void *)address;

        while (argc--)
        {
            *scan++ = strtoull(*argv++, 0, 16);
        }

        if (process_vm_writev(pid, local, 1, other, 1, 0) < 0)
        {
            perror("process_vm_writev");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Specify 'r' or 'w' for read or write\n");
        usage(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
