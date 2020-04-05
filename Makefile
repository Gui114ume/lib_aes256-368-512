CC=gcc
CFLAGS=-g -Wall -O0
LDFLAGS=-lm -L./ -lsha2
EXEC=

all: libsha2.so mysha2

libsha2.so: library.c
	$(CC) -shared -fPIC $^ -o $@ $(CFLAGS)

mysha2.o: main.c
	$(CC) -c $^ -o $@ $(CFLAGS)

mysha2 : mysha2.o libsha2.so
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	 rm -rf *.o file~ libsha2.so mysha2
