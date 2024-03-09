CC = gcc
CFLAGS = -m32 -g -Wall

encoder:
	$(CC) $(CFLAGS) -c -o encoder.o encoder.c
	$(CC) $(CFLAGS) -o encoder encoder.o

clean:
	rm -f encoder.o encoder
