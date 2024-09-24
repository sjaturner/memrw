CC=gcc
CFLAGS=-Wall -Wextra
test: test.c
memrw: memrw.c
all: test memrw
.PHONY:clean
clean:
	rm -f test memrw
