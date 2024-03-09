CC=gcc
ASM=nasm
LD=ld

CFLAGS=-m32 -Wall -ansi -c -nostdlib -fno-stack-protector
ASMFLAGS=-f elf32
LDFLAGS=-m elf_i386

all: clean task0

task0: start.o main.o util.o hello.o 
	$(LD) $(LDFLAGS) start.o main.o util.o hello.o -o task0

hello.o: hello.s
	$(ASM) $(ASMFLAGS) hello.s -o hello.o

start.o: start.s
	$(ASM) $(ASMFLAGS) start.s -o start.o

main.o: main.c
	$(CC) $(CFLAGS) main.c -o main.o
	
util.o: util.c
	$(CC) $(CFLAGS) util.c -o util.o

clean:
	rm -f task0 hello.o start.o main.o util.o
