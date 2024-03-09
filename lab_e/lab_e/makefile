CC = gcc
CFLAGS = -m32 -Wall -g

all: clean myELF 

myELF:
	$(CC) $(CFLAGS) -c -o myELF.o myELF.c
	$(CC) $(CFLAGS) -o myELF myELF.o

clean:
	rm -f myELF 