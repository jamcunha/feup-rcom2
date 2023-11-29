CC = gcc
CFLAGS = -Wall

all: main
	$(CC) $(CFLAGS) -o main main.o

main: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f main.o main
