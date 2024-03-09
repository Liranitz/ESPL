section .data
    hello_world db 'hello world', 10  ; Define the string to print, followed by newline
    hello_world_length equ $ - hello_world  ; Calculate the length of the string

section .text
    global _start  ; Entry point for the program

_start:
    ; Write the string to stdout
    mov eax, 4       ; System call number for sys_write
    mov ebx, 1       ; File descriptor for stdout
    mov ecx, hello_world  
    mov edx, hello_world_length
    int 0x80         ; Invoke the kernel

    ; Exit the program
    mov eax, 1       ; System call number for sys_exit
    xor ebx, ebx     ; Exit code 0
    int 0x80         ; Invoke the kernel