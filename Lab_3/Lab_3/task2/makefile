all: clean task2
task2:
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o 
	ld -m elf_i386 start.o main.o util.o -o task2
	
clean:
	rm -f *.o task2