SOURCES = start.s hello.s util.c main.c

all: clean task0 task0_hello

task0:	
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o
	ld -m elf_i386 start.o main.o util.o -o task0

task0_hello:
	nasm -f elf32 hello.s -o hello.o
	ld -m elf_i386 hello.o util.o -o task0_hello

clean:
	rm -f *.o task0 task0_hello