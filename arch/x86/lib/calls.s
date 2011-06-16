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

global call_open2
call_open2:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x55
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

global call_read
call_read:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x02
    mov ebx, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_write
call_write:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x03
    mov ebx, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
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

