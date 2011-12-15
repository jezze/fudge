extern main
extern call_exit

global _start
_start:
    mov eax, [esp + 4]
    mov ebx, [esp + 8]
    push ebx
    push eax
    call main
    add esp, 8
    call call_exit

