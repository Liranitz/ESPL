CC = gcc
CFLAGS = -m32 -Wall -g

all: clean loader

loader:
	nasm -f elf32 start.s -o start.o
	nasm -f elf32 startup.s -o startup.o
	$(CC) $(CFLAGS) -c loader.c -o loader.o
	ld -o loader loader.o start.o startup.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2


clean:
	rm -f loader  loader.o