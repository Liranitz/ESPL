CC = gcc
CFLAGS = -m32 -Wall -g

all: clean myshell mypipeline

myshell:
	$(CC) $(CFLAGS) -c -o myshell.o myshell.c
	$(CC) $(CFLAGS) -c -o LineParser.o LineParser.c
	$(CC) $(CFLAGS) -o myshell myshell.o LineParser.o
	$(CC) $(CFLAGS) -c -o looper.o looper.c
	$(CC) $(CFLAGS) -o looper looper.o

mypipeline:
	$(CC) $(CFLAGS) -c -o mypipeline.o mypipeline.c
	$(CC) $(CFLAGS) -o mypipeline mypipeline.o


clean:
	rm -f looper myshell mypipeline mynewshell df *.o
