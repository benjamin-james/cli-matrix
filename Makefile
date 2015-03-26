CC=gcc
CFLAGS=-g -Wall -march=native
all: src/main.c array.o
	$(CC) $(CFLAGS) -O2 src/main.c array.o -o cli-matrix
array.o: src/array.c
	$(CC) $(CFLAGS) -c -O3 src/array.c -o array.o
clean:
	rm array.o
	rm cli-matrix
