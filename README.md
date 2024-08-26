# memrw A Tool For Reading and Writing to Memory in Other Processes

If you are root on a Linux system you are free to read and alter
the memory of other processes using the following system calls:
process\_vm\_writev and process\_vm\_readv.

The memrw program is a small command line program which wraps these 
two system calls and allows arbitrary reads and writes to different 
processes, provided you know the PID of the target.

Clearly, some care is required while using this program.

## Usage

If you run memrw with no arguments you will see the following 
usage information:

    read usage:
        ./memrw pid address r count
    where:
        pid is the PID of the process to read memory from
        address is the virtual address within the process where the read starts, hexadecimal addresses require an 0x prefix
        count is the number of bytes to read, these will be will be emitted as octets in hexadecimal on a single line
    write usage:
        ./memrw pid address w first_octet second_octet ...
    where:
        pid is the PID of the process to write into
        address is the virtual address within the process where the write will start, hexadecimal addresses require an 0x prefix
        first_octet, etc. octets to be written to the start address and subsequent addresses the octets comprise two hexadecimal nibbles followed by space with no 0x prefix

## Building the Programs

Type:

    make all

## Example

The test program in this directory contains an array in memory and a loop which simply 
dumps out the contents of that array.

When executed, you will see something like this:

    :; ./test
    array is at 0x55e06a015030
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    ...

The contents of the array are printed out as octets in hexadecimal. The
array is printed every second, for reasons which will become apparent.

The address of that array is also printed out at the start of the 
program.

To inspect the array using memrw, do the following - being careful to 
copy the pointer printed at the start of the test program output:

    :; sudo ./memrw $(pidof test) 0x55e06a015030 r 0x10
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff

As you can see, we're reading from the address space of the test program.

We can modify the contents of that array like this:

    :; sudo ./memrw $(pidof test) 0x55e06a015030 w 12 34 56 78

At which point, the output of the test program will alter:

    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    12 34 56 78 05 06 07 08 f8 f9 fa fb fc fd fe ff  <- write occurred before here
    12 34 56 78 05 06 07 08 f8 f9 fa fb fc fd fe ff
    12 34 56 78 05 06 07 08 f8 f9 fa fb fc fd fe ff

So the first four values are now different.

## Disabling Address Space Randomisation for Predictable Addresses

If you run test a few times you'll see that the array location changes.
This is a security feature of Linux and intentionally makes it harder
to use tools like memrw to read from or alter running programs. That's
clearly advantageous but for debugging purposes you can disable that on
the command line:

    :; sudo setarch -R ./test
    array is at 0x555555558030
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    ^C
    :; sudo setarch -R ./test
    array is at 0x555555558030
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    ^C
    :; sudo setarch -R ./test
    array is at 0x555555558030
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    01 02 03 04 05 06 07 08 f8 f9 fa fb fc fd fe ff
    ^C

The array address always comes out at the same point. And in fact that
address is predictable and can be calculated from the symbol information:

    :; sudo ./memrw $(pidof test) $(printf "0x%016x\n" $(( 0x$(nm test | grep -w array | cut -d' ' -f1) + 0x555555554000 ))) r 4
    01 02 03 04

Breaking that down into stages: nm gets the symbols from test; the
grep extracts the entry for the array variable and cut retrieves the
address part of that. We prefix a 0x because it's hexadecimal and for
some reason nm doesn't do that. We then add the 0x555555554000 offset
which is the required adjustment for Linux (amd64). When we read bytes
at that address we're looking at the array again.

While that looks a little complicated, it's easy to incorporate into
a script.

## Applications

This technique is best suited to inspecting global and static variables.
Having said that, often much can be inferred from that state information.

If you have programs which you expect to need to debug, and you are on
a secure system then perhaps setarch -R is an option - although be aware
of the security risks of side stepping the address space randomisation.

In addition, if you are unable to use setarch -R but print out the address 
of a single global variable then you can use the offsets from that variable 
in nm to see the remainder.

Rather than periodically printing data structures for debug, you can place
copies of key structures into globals and inspect them with this program.

The following repos contain software for working with structures:
https://github.com/sjaturner/cdump and, for data dumped as octets in
hexadecimal https://github.com/sjaturner/arrange might also be helpful.
