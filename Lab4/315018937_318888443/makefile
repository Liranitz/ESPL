CC = gcc
CFLAGS = -m32 -Wall -g

all: clean task1 task4

task1:
	$(CC) $(CFLAGS) -c -o task1.o task1.c
	$(CC) $(CFLAGS) -o task1 task1.o

task4:
	gcc -m32 -fno-pie -fno-stack-protector -o task4 task4.c

clean:
	rm -f task1 task4 *.o
