CFLAGS=-std=c99

default: all

all: example

example: example.c ht.c siphash.c
	cc ${CFLAGS} -o example example.c ht.c siphash.c

clean:
	-rm *.o example
