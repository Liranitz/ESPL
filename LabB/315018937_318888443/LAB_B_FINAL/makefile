CC = gcc
CFLAGS = -m32 -Wall -g

all: AntiVirus

AntiVirus:
	$(CC) $(CFLAGS) -c -o AntiVirus.o AntiVirus.c
	$(CC) $(CFLAGS) -o AntiVirus AntiVirus.o

clean:
	rm -f AntiVirus *.o
