all: clean task1

task1:	
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	ld -m elf_i386 start.o util.o -o task1

clean:
	rm -f *.o task1