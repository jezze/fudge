global call_vfs_walk
call_vfs_walk:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x12
    mov ecx, [ebp + 8]
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

global call_vfs_find
call_vfs_find:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov eax, 0x13
    mov esi, [ebp + 8]
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
    mov eax, 0x50
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
    mov eax, 0x51
    int 0x80
    pop edi
    pop esi
    pop ebp
    ret

