global call_open
call_open:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x00
    mov esi, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_close
call_close:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x01
    mov ebx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_map
call_map:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x10
    mov ebx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_halt
call_halt:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x20
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_reboot
call_reboot:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x21
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

