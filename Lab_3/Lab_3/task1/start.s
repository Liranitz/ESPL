section .rodata
    NEW_LINE_CHAR_e: db 10
section .bss
    char: resb 1
section .text
global _start
global sys
extern strlen
extern printf
section .data
    SYS_WRITE_e dd 4
    OUT_e dd 1
    InFile: dd 0
    OutFile: dd 1


_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    push ebp
    mov ebp, esp
    mov edi, [ebp+8] ; Get first argument ac
    mov esi, [ebp+12] ; Get 2nd argument av
    loop:
        pushad

        mov ebx, [esi]
        cmp byte [ebx], '-'
        jne print_only

        mov eax, [ebx + 1]      ; Load the byte at the next memory address after ebx
        cmp al, 'i'             ; Compare the byte with 'i'
        je open_input_file       ; Jump to input_function if it's equal
        cmp al, 'o'             ; Compare the byte with 'o'
        je open_output      ; Jump to output_function if it's equal

    print_only:
        push dword[esi]
        call strlen
        pop ecx
        add esp, 4

        pushad
        push eax
        push dword[esi]
        push dword[OUT_e]
        push dword[SYS_WRITE_e]
        call system_call
        add esp, 16
        popad

        ;print new line
        pushad
        push 1
        push NEW_LINE_CHAR_e
        push dword[OUT_e]
        push dword[SYS_WRITE_e]
        call system_call
        add esp, 16
        popad
        
    inc_counter:
        add esi, 4
        dec edi ;edi++
        jnz loop ;jump not equal 

; TODO : LIRAN
encoder:
    read_input:
        pushad
        push 1
        push char
        push dword[InFile]
        push 3
        call system_call
        add esp, 16
        pushad
        cmp eax, 0
        jle in_file_close
    encode:
        cmp byte[char], 'A'
        jl print_output
        cmp byte[char], 'z'
        jg print_output
        inc byte[char]
    print_output:
        pushad
        push 1
        push char
        push dword[OutFile]
        push 4
        call system_call
        add esp, 16
        pushad
    jmp read_input
in_file_close:
    cmp dword[InFile], 0
    je close_input
out_file_close:
    cmp dword[OutFile], 1
    je close_output
    
    exit_program:
        mov     eax, 1
        mov     ebx, 0
        int     0x80

open_input_file:
    add ebx, 2
    pushad
    push 0
    push 0 ; push 0 for read only
    push ebx
    push 5
    call system_call
    add esp, 16
    pushad
    cmp eax, 0      ;0 is eof, -1 is error, finish loop if return value is smaller or equal to 0
    jl exit_program
    mov dword[InFile], eax
    jmp inc_counter


open_output:
    add ebx, 2
    
    pushad
    push 644o
    push 1101o 
    push ebx
    push 5
    call system_call
    add esp, 16
    pushad

    cmp eax, 0 
    jl exit_program
    mov dword[OutFile], eax
    jmp inc_counter

close_input:
    pushad
    push dword[InFile]
    push 6
    call system_call
    add esp, 8
    pushad
    
    jmp out_file_close
close_output:
    pushad
    push dword[OutFile]
    push 6
    call system_call
    add esp, 8
    pushad

    jmp exit_program
