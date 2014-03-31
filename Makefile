CC=gcc
CFLAGS=-Wall -g

default: all

core:
	$(CC) $(CFLAGS) -o topswapcore topswapcore.c

all: core
	$(CC) $(CFLAGS) -o topswap topswap.c

clean:
	rm -f a.out *.o topswap topswapcore
