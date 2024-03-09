CC = gcc
CFLAGS = -m32 -Wall -g

all: clean myshell mypipe

myshell:
	$(CC) $(CFLAGS) -c -o myshell.o myshell.c
	$(CC) $(CFLAGS) -c -o LineParser.o LineParser.c
	$(CC) $(CFLAGS) -o myshell myshell.o LineParser.o
	$(CC) $(CFLAGS) -c -o looper.o looper.c
	$(CC) $(CFLAGS) -o looper looper.o

mypipe:
	$(CC) $(CFLAGS) -c -o mypipe.o mypipe.c
	$(CC) $(CFLAGS) -o mypipe mypipe.o

clean:
	rm -f looper myshell mypipe *.o
