section .data
    hello db 'Hello, world!', 0xA ; 'Hello, world!' followed by newline character
    hello_len equ $ - hello ; length of the string

section .text
    global _start

_start2:
    ; write message to stdout
    mov eax, 4         ; syscall number for sys_write
    mov ebx, 1         ; file descriptor 1 (stdout)
    mov ecx, hello     ; pointer to the message to print
    mov edx, hello_len ; length of the message
    int 0x80           ; call the kernel

    ; exit program
    mov eax, 1         ; syscall number for sys_exit
    xor ebx, ebx       ; exit code 0
    int 0x80           ; call the kernel